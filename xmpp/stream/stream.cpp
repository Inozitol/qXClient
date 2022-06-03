#include "stream.h"

Stream::Stream(std::shared_ptr<Account> account, std::shared_ptr<Server> server)
    : _acc(account),
      _srv(server)
{
    initSocket();
    _reader.setDevice(_socket);

    connect(this, &Stream::coreEstablished, this, [this](){_contacts = new ContactTreeModel(_acc->jid(), this);});
    connect(this, &Stream::coreEstablished, this, &Stream::queryRoster);
    connect(this, &Stream::coreEstablished, this, [this](){ advertisePresence(true); });
}

Stream::~Stream(){
    delete(_socket);
    delete(_contacts);
}

jidbare_t Stream::accountJid(){
    return _acc->jid();
}

void Stream::setDescriptor(qintptr descriptor){
    _descriptor = descriptor;
}

ContactTreeModel *Stream::getContactsModel(){
    return _contacts;
}

ChatChain* Stream::getChatChain(const jidbare_t& jid){
    _chainMutex.lock();
    if(_chatChains.count(jid)){
        _chainMutex.unlock();
        return _chatChains.at(jid);
    }
    _chainMutex.unlock();
    return nullptr;
}

void Stream::reconnectSecure(){
    _socket->startClientEncryption();
    _socket->write(getHeader() + "\r\n");
}

void Stream::connectInsecure(){
    _socket->connectToHost(_srv->jid().domain, _srv->port());
    _socket->write(getHeader() + "\r\n");
}

void Stream::initSocket(){
    if(_socket) return;

    _socket = new QSslSocket(this);

    connect(_socket,  &QSslSocket::stateChanged,
            this,       &Stream::sslSockStateChange);

    connect(_socket,  &QSslSocket::readyRead,
            this,       &Stream::readData);

    connect(_socket,  &QSslSocket::errorOccurred,
            this,       &Stream::socketError);

    connect(_socket,  QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            this,       &Stream::sslErrors);

    connect(_socket,  &QSslSocket::disconnected,
            this,       &Stream::disconnectHandle);

    connect(_socket,  &QSslSocket::encrypted,
            this,       [](){qDebug() << "SSL Encrypted";});
}

void Stream::sslSockStateChange(QAbstractSocket::SocketState state){
    qDebug() << "State change:" << state;
}

void Stream::stateINIT(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger&){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::stream:
            _srvStreamId = QByteArray::fromBase64(_reader.attributes().value("id").toUtf8());
            _readerState = ReaderState::STREAM;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateSTREAM(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::features:
            _readerState = ReaderState::FEATURES;
            break;
        case XMLWord::iq:
        {
            auto ft_bind = std::static_pointer_cast<FeatureBind>(_features.value(FeatureType::RESOURCEBIND));

            InfoQuery iq = InfoQuery(_reader);
            token = _reader.tokenType();
            name = _reader.name().toUtf8();
            qDebug() << "IQ:" << iq.str();

            QByteArray id = iq.getId();

            switch(_iqWaiting.at(id)){
            case IQPurpose::FEATURE:
                trigger |= ReaderTrigger::FEATURE;
                break;
            case IQPurpose::ROSTER:
                trigger |= ReaderTrigger::INFOQUERY;
                break;
            }
            _iqResults.insert(std::make_pair(std::move(id), std::move(iq)));
        }
            break;
        case XMLWord::presence:
        {
            Presence presence(_reader);
            qDebug() << "Presence:" << presence.str();
            _contacts->insertPresence(presence);
        }
            break;
        case XMLWord::message:
        {
            Message message(_reader);
            qDebug() << "Message from:" << message.getFrom() << "reads" << message.getBody();
            qDebug() << message.str();
            jidfull_t jid = message.getFrom();
            if(!_chatChains.count(jid)){
                ChatChain* chain = new ChatChain(this, jid, this);
                connect(chain, &ChatChain::receivedMessage,
                        this, &Stream::receivedMessage);
                connect(chain, &ChatChain::sendMessage,
                        this, &Stream::sendMessage);
                _chatChains.insert({jid, chain});
            }
            _chatChains.at(jid)->addMessage(std::move(message));
        }
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case XMLWord::stream:
            // TODO Start disconnect process
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateFEATURES(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::starttls:
            _features.insert(FeatureType::STARTTLS, std::make_shared<FeatureSTARTTLS>());
            trigger |= ReaderTrigger::FEATURE;
            _readerState = ReaderState::STARTTLS;
            break;
        case XMLWord::mechanisms:
            _features.insert(FeatureType::SASL, std::make_shared<FeatureSASL>());
            trigger |= ReaderTrigger::FEATURE;
            _readerState = ReaderState::SASL;
            break;
        case XMLWord::bind:
            _features.insert(FeatureType::RESOURCEBIND, std::make_shared<FeatureBind>());
            trigger |= ReaderTrigger::FEATURE;
            _readerState = ReaderState::RESOURCEBIND;
            break;
        default:
            break;
        }
        break;

    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case XMLWord::features:
            _readerState = ReaderState::STREAM;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateSTARTTLS(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::required:
            _features.value(FeatureType::STARTTLS)->required = true;
            break;
        case XMLWord::proceed:
            std::static_pointer_cast<FeatureSTARTTLS>(_features.value(FeatureType::STARTTLS))->state = StateSTARTTLS::PROCEED;
            trigger |= ReaderTrigger::FEATURE;
            _readerState = ReaderState::STREAM;
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case XMLWord::starttls:
            _readerState = ReaderState::FEATURES;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

void Stream::stateSASL(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::mechanism:
        {
            token = _reader.readNext();
            name = _reader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(_features.value(FeatureType::SASL));
            feature_sasl->srv_mechanisms.insert(_reader.text().toString());
            trigger |= ReaderTrigger::FEATURE;
        }
            break;
        case XMLWord::challenge:
        {
            token = _reader.readNext();
            name = _reader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(_features.value(FeatureType::SASL));
            feature_sasl->challenge = QByteArray::fromBase64(_reader.text().toUtf8());
            feature_sasl->state = StateSASL::CHALLENGE;
            trigger |= ReaderTrigger::FEATURE;
            _readerState = ReaderState::STREAM;
        }
            break;
        case XMLWord::success:
        {
            token = _reader.readNext();
            name = _reader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(_features.value(FeatureType::SASL));
            feature_sasl->server_sig = QByteArray::fromBase64(_reader.text().toUtf8());
            feature_sasl->state = StateSASL::SUCCESS;
            trigger |= ReaderTrigger::FEATURE;
            _readerState = ReaderState::STREAM;
        }
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case XMLWord::mechanisms:
            _readerState = ReaderState::FEATURES;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateRESOURCEBIND(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::required:
            _features.value(FeatureType::RESOURCEBIND)->required = true;
            trigger |= ReaderTrigger::FEATURE;
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case XMLWord::bind:
            _readerState = ReaderState::STREAM;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::readData(){
    ReaderTrigger trigg = {};

    while(!_reader.atEnd()){
        QXmlStreamReader::TokenType token = _reader.readNext();
        QByteArray name = _reader.name().toUtf8();
        if(token == QXmlStreamReader::Invalid){
            qDebug() << "ERROR: " << _reader.error() << _reader.errorString();
            token = _reader.readNext();
            name = _reader.name().toUtf8();
        }
        switch(_readerState){

        case ReaderState::INIT:
            stateINIT(token,name,trigg);
            break;
        case ReaderState::STREAM:
            stateSTREAM(token,name,trigg);
            break;
        case ReaderState::FEATURES:
            stateFEATURES(token,name,trigg);
            break;
        case ReaderState::STARTTLS:
            stateSTARTTLS(token,name,trigg);
            break;
        case ReaderState::SASL:
            stateSASL(token,name,trigg);
            break;
        case ReaderState::RESOURCEBIND:
            stateRESOURCEBIND(token,name,trigg);
            break;
        }
        qDebug() << "TOKEN: " << token << _reader.text() << name << _reader.namespaceUri();
    }
    if(trigg & ReaderTrigger::FEATURE){
        processFeatures();
    }
    if(trigg & ReaderTrigger::INFOQUERY){
        processInfoQuery();
    }
}

void Stream::processFeatures(){
    bool erase_feature;
    for(auto it = _features.constBegin(); it != _features.constEnd();){
        erase_feature = false;
        switch(it.key()){
        case FeatureType::STARTTLS:
        {
            qDebug() << "Feature STARTTLS | Required: " << it.value()->required;
            auto ft_starttls = std::static_pointer_cast<FeatureSTARTTLS>(it.value());
            switch(ft_starttls->state){
            case StateSTARTTLS::ASK:
                _socket->write(getSTARTTLS() + "\r\n");
                _readerState = ReaderState::STARTTLS;
                break;
            case StateSTARTTLS::PROCEED:
                _reader.clear();
                _reader.setDevice(_socket);
                reconnectSecure();
                _featuresActive |= FeatureType::STARTTLS;
                erase_feature  = true;
                _readerState = ReaderState::INIT;
                break;
            case StateSTARTTLS::FAILURE:
                _socket->disconnectFromHost();
                break;
            }
        }
            break;
        case FeatureType::SASL:
        {
            qDebug() << "Feature SASL | Required: " << it.value()->required;
            auto ft_sasl = std::static_pointer_cast<FeatureSASL>(it.value());
            switch(ft_sasl->state){
            case StateSASL::AUTH:
                for(const auto& sasl : _saslSupport){
                    if(ft_sasl->srv_mechanisms.contains(SASL2StrMapper.value(sasl))){
                        ft_sasl->mechanism = sasl;
                        _scramGenerator = std::make_shared<SASL::SCRAMGenerator>(sasl, _acc->credentials());
                        _scramGenerator->setSCRAMFlag(SASL::SCRAMGenerator::CHANNEL_FLAG::N);
                        _scramGenerator->writeAttribute(SASL::SCRAMGenerator::Attributes::NAME, _acc->jid().local);
                        _scramGenerator->writeAttribute(SASL::SCRAMGenerator::Attributes::NONCE_CLIENT, Utils::getRandomString(NONCE_INIT_LEN));
                        _socket->write(getAuth() + "\r\n");
                        _readerState = ReaderState::SASL;
                        break;
                    }
                    // TODO no SASL found
                }
                break;
            case StateSASL::CHALLENGE:
            {
                _scramGenerator->loadChallenge(ft_sasl->challenge);
                _socket->write(getResponse() + "\r\n");
                _readerState = ReaderState::SASL;
            }
                break;
            case StateSASL::SUCCESS:
            {
                if(!_scramGenerator->isServerSigValid(ft_sasl->server_sig)){
                    qDebug() << "Invalid server signature";
                    // TODO invalid server signature
                }
                _reader.clear();
                _reader.setDevice(_socket);
                _socket->write(getHeader() + "\r\n");
                _featuresActive |= FeatureType::SASL;
                erase_feature  = true;
                _readerState = ReaderState::INIT;
            }
                break;
            }

        }
            break;
        case FeatureType::RESOURCEBIND:
        {
            qDebug() << "Feature RESOURCEBIND | Required: " << it.value()->required;
            auto ft_bind = std::static_pointer_cast<FeatureBind>(it.value());
            if(ft_bind->query_id.isEmpty()){
                InfoQuery iq = InfoQuery();
                iq.setId();
                iq.setType("set");

                QByteArray id = iq.getId();
                ft_bind->query_id = id;
                _iqWaiting.insert({id, IQPurpose::FEATURE});

                QDomElement bind = iq.createElement("bind");
                bind.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");

                iq.insertNode(bind);

                _socket->write(iq.str() + "\r\n");
            }else{
                InfoQuery iq_result = _iqResults.at(ft_bind->query_id);
                _iqWaiting.erase(_iqWaiting.find(ft_bind->query_id));
                _iqResults.erase(_iqResults.find(ft_bind->query_id));

                switch(word2int(iq_result.getType())){
                case XMLWord::result:
                {
                    QDomNode bind_node = iq_result.root().firstChild();
                    QDomElement jid_elem = bind_node.firstChildElement();

                    jidfull_t jid = jid_elem.text().toUtf8();
                    _acc->setJid(jid);
                    _featuresActive |= FeatureType::RESOURCEBIND;
                    erase_feature = true;
                    emit coreEstablished();
                }
                    break;
                case XMLWord::error:
                    // TODO Handle bind errors
                    break;
                default:
                    break;
                }
            }
        }
            break;
        case FeatureType::UNKNOWN:
            qDebug() << "Feature UNKNOWN | Required: " << it.value()->required;

            break;
        }
        if(erase_feature){
            it = _features.erase(it);
        }else{
            ++it;
        }
    }
}

void Stream::processInfoQuery(){
    auto it = _iqResults.cbegin();
    while(it != _iqResults.cend()){
        QByteArray id = it->first;
        InfoQuery iq = it->second;
        IQPurpose purpose = _iqWaiting.at(id);

        switch(purpose){
        case IQPurpose::ROSTER:
        {
            QDomNode query_node = iq.root().firstChild();
            QDomNodeList item_list = query_node.childNodes();
            for(int i=0; i<item_list.length(); i++){
                QDomElement curr_item = item_list.at(i).toElement();
                rosteritem_t roster(curr_item);
                _contacts->setRoster(roster);
                if(!_chatChains.count(roster.jid)){
                    ChatChain* chain = new ChatChain(this, roster.jid, this);
                    connect(chain, &ChatChain::receivedMessage,
                            this, &Stream::receivedMessage);
                    connect(chain, &ChatChain::sendMessage,
                            this, &Stream::sendMessage);
                    _chatChains.insert({roster.jid, chain});
                }
            }
        }
            break;
        default:
            break;
        }
        _iqWaiting.erase(_iqWaiting.find(id));
        it = _iqResults.erase(it);
    }
}

void Stream::queryRoster(){
    InfoQuery iq;
    iq.setFrom(_acc->jid().str());
    iq.setId();
    iq.setType("get");

    _iqWaiting.insert({iq.getId(), IQPurpose::ROSTER});

    QDomElement query = iq.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:roster");

    iq.insertNode(query);

    _socket->write(iq.str() + "\r\n");
}

void Stream::advertisePresence(bool status){
    QByteArray presence_tag = getPresence(status);
    _socket->write(presence_tag + "\r\n");
}

void Stream::disconnectHandle(){
    emit finished();
}

void Stream::sendMessage(Message& msg){
    qDebug() << "Message to:" << msg.getTo() << "reads" << msg.getBody();
    qDebug() << msg.str();
    _socket->write(msg.str() + "\r\n");
}

void Stream::socketError(QAbstractSocket::SocketError){
    qDebug() << "Socket error:" << _socket->errorString();
}

void Stream::sslErrors(const QList<QSslError> &errors){
    for (const auto &error : errors)
        qDebug() << "SSL Error: " << error.errorString();
}

QByteArray Stream::getHeader(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartDocument();
    writer.writeStartElement("stream:stream");
    writer.writeAttribute("from",     _acc->jid().str());
    writer.writeAttribute("to",       _srv->jid().domain);
    writer.writeAttribute("version",  "1.0");
    writer.writeAttribute("xml:lang", "en");
    writer.writeAttribute("xmlns",    "jabber:client");
    writer.writeAttribute("xmlns:stream", "http://etherx.jabber.org/streams");
    arr.append('>');
    return arr;
}

QByteArray Stream::getHeaderEnd(){
    QByteArray arr = "</stream:stream>";
    return arr;
}

QByteArray Stream::getSTARTTLS(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("starttls");
    writer.writeAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");
    writer.writeEndElement();
    return arr;
}

QByteArray Stream::getProceed(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("proceed");
    writer.writeAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-tls");
    writer.writeEndElement();
    return arr;
}

QByteArray Stream::getAuth(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("auth");
    writer.writeAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    writer.writeAttribute("mechanism", SASL2StrMapper.value(_scramGenerator->GetSASL()));
    writer.writeCharacters(_scramGenerator->getInitResponse().toBase64());
    writer.writeEndElement();
    return arr;
}

QByteArray Stream::getResponse(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("response");
    writer.writeAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    writer.writeCharacters(_scramGenerator->getChallengeResponse().toBase64());
    writer.writeEndElement();
    return arr;
}
QByteArray Stream::getPresence(bool status){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("presence");
    if(!status){
        writer.writeAttribute("type","unavailable");
    }
    writer.writeEndElement();
    return arr;
}

Stream::ReaderTrigger operator|(Stream::ReaderTrigger l, Stream::ReaderTrigger r){
    return static_cast<Stream::ReaderTrigger>(
                static_cast<std::underlying_type_t<Stream::ReaderTrigger>>(l) |
                static_cast<std::underlying_type_t<Stream::ReaderTrigger>>(r)
                );
}

Stream::ReaderTrigger operator|=(Stream::ReaderTrigger& l, Stream::ReaderTrigger r){
    l = l | r;
    return l;
}


bool operator&(Stream::ReaderTrigger l, Stream::ReaderTrigger r){
    return static_cast<Stream::ReaderTrigger>(
                static_cast<std::underlying_type_t<Stream::ReaderTrigger>>(l) &
                static_cast<std::underlying_type_t<Stream::ReaderTrigger>>(r)
                ) == r;
}
