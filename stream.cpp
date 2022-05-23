#include "stream.h"

Stream::Stream(std::shared_ptr<Account> client, std::shared_ptr<Server> server)
    : _acc(client),
      _srv(server)
{
    initSocket();
    _reader.setDevice(_socket);
    qDebug() << "Stream created"
             << "\nclient jid: " << _acc->jid()
             << "\nserver jid: " << _srv->jid();
}

Stream::~Stream(){
    delete(_socket);
}

void Stream::reconnectSecure(){
    _socket->startClientEncryption();
    _socket->write(getHeader() + "\r\n");
}

void Stream::connectInsecure(){
    _socket->connectToHost(_srv->jid().domain, 5222);
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

void Stream::stateINIT(QXmlStreamReader::TokenType& token, QByteArray& name, bool&){
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

void Stream::stateSTREAM(QXmlStreamReader::TokenType& token, QByteArray& name, bool&){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::features:
            _readerState = ReaderState::FEATURES;
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

void Stream::stateFEATURES(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::starttls:
            _features.insert(FeatureType::STARTTLS, std::make_shared<FeatureSTARTTLS>());
            ft_change = true;
            _readerState = ReaderState::STARTTLS;
            break;
        case XMLWord::mechanisms:
            _features.insert(FeatureType::SASL, std::make_shared<FeatureSASL>());
            ft_change = true;
            _readerState = ReaderState::SASL;
            break;
        case XMLWord::bind:
            _features.insert(FeatureType::RESOURCEBIND, std::make_shared<FeatureBind>());
            ft_change = true;
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

void Stream::stateSTARTTLS(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::required:
            _features.value(FeatureType::STARTTLS)->required = true;
            break;
        case XMLWord::proceed:
            std::static_pointer_cast<FeatureSTARTTLS>(_features.value(FeatureType::STARTTLS))->state = StateSTARTTLS::PROCEED;
            ft_change = true;
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

void Stream::stateSASL(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::mechanism:
        {
            token = _reader.readNext();
            name = _reader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(_features.value(FeatureType::SASL));
            feature_sasl->srv_mechanisms.insert(_reader.text().toString());
            ft_change = true;
        }
            break;
        case XMLWord::challenge:
        {
            token = _reader.readNext();
            name = _reader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(_features.value(FeatureType::SASL));
            feature_sasl->challenge = QByteArray::fromBase64(_reader.text().toUtf8());
            feature_sasl->state = StateSASL::CHALLENGE;
            ft_change = true;
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
            ft_change = true;
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

void Stream::stateRESOURCEBIND(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case XMLWord::required:
            _features.value(FeatureType::RESOURCEBIND)->required = true;
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
    bool ft_change = false;

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
            stateINIT(token,name,ft_change);
            break;
        case ReaderState::STREAM:
            stateSTREAM(token,name,ft_change);
            break;
        case ReaderState::FEATURES:
            stateFEATURES(token,name,ft_change);
            break;
        case ReaderState::STARTTLS:
            stateSTARTTLS(token,name,ft_change);
            break;
        case ReaderState::SASL:
            stateSASL(token,name,ft_change);
            break;
        case ReaderState::RESOURCEBIND:
            stateRESOURCEBIND(token,name,ft_change);
            break;
        }
        qDebug() << "TOKEN: " << token << _reader.text() << name << _reader.namespaceUri();
    }
    if(ft_change){
        processFeatures();
    }
}

void Stream::processFeatures(){
    bool erase_feature ;
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
                        _scramGenerator->writeAttribute(SASL::SCRAMGenerator::Attributes::NAME, _acc->jid().local.toUtf8());
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
            switch(ft_bind->state){
            case StateBind::GENERATE:
                InfoQuery iq = InfoQuery();
                iq.setId();
                ft_bind->query_id = iq.getId();
                QDomElement bind;
                bind.setTagName("bind");
                bind.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");
                iq.appendChild(bind);
                QTextStream ts;
                ts << iq.toText();
                qDebug() << ts.readAll();
                break;
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

void Stream::disconnectHandle(){
    emit finished();
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
