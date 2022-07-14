#include "stream.h"

Stream::Stream(const Account& account, const Server& server)
    : m_uptrAccount(std::make_unique<Account>(account)),
      m_uptrServer(std::make_unique<Server>(server))
{
    initSocket();
    initSignals();
    m_xmlReader.setDevice(m_ptrSocket);
}

Stream::Stream(Account&& account, Server&& server)
    : m_uptrAccount(std::make_unique<Account>(std::move(account))),
      m_uptrServer(std::make_unique<Server>(std::move(server)))
{
    initSocket();
    initSignals();
    m_xmlReader.setDevice(m_ptrSocket);
}

Stream::~Stream(){
    delete(m_ptrSocket);
    delete(m_ptrContactsModel);
}

jidbare_t Stream::accountJid(){
    return m_uptrAccount->jid();
}

ContactTreeModel *Stream::contactsModel(){
    return m_ptrContactsModel;
}

ChatChain* Stream::chatChain(const jidbare_t& jid){
    if(m_umapChatChains.count(jid)){
        return m_umapChatChains.at(jid);
    }
    return nullptr;
}

void Stream::connectInsecure(){
    m_ptrSocket->connectToHost(m_uptrServer->jid().domain, m_uptrServer->getPort());
    m_ptrSocket->write(getHeader() + "\r\n");
}

void Stream::initSocket(){
    if(m_ptrSocket) return;

    m_ptrSocket = new QSslSocket(this);

    connect(m_ptrSocket,  &QSslSocket::stateChanged,
            this,     &Stream::sslSockStateChange);

    connect(m_ptrSocket,  &QSslSocket::readyRead,
            this,     &Stream::readData);

    connect(m_ptrSocket,  &QSslSocket::errorOccurred,
            this,     &Stream::socketError);

    connect(m_ptrSocket,  QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            this,     &Stream::sslErrors);

    connect(m_ptrSocket,  &QSslSocket::disconnected,
            this,     &Stream::initDisconnect);

    connect(m_ptrSocket,  &QSslSocket::encrypted,
            this,     [](){qDebug() << "SSL Encrypted";});
}

void Stream::initSignals(){
    connect(this, &Stream::coreEstablished, this, [this](){ m_isCoreEstablished = true; });
    connect(this, &Stream::coreEstablished, this, [this](){ m_ptrContactsModel = new ContactTreeModel(m_uptrAccount->jid(), this); });
    connect(this, &Stream::coreEstablished, this, &Stream::queryRoster);
    connect(this, &Stream::coreEstablished, this, &Stream::queryDisco);
    connect(this, &Stream::coreEstablished, this, [this](){ advertisePresence(true); });
}

void Stream::reconnectSecure(){
    m_ptrSocket->startClientEncryption();
    m_ptrSocket->write(getHeader() + "\r\n");
}

void Stream::sslSockStateChange(QAbstractSocket::SocketState state){
    qDebug() << "State change:" << state;
}

void Stream::stateINIT(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>&){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case IntFromString::stream:
            m_streamID = QByteArray::fromBase64(m_xmlReader.attributes().value("id").toUtf8());
            m_xmlReaderState = ReaderState::STREAM;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateSTREAM(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case IntFromString::features:
            m_xmlReaderState = ReaderState::FEATURES;
            break;
        case IntFromString::iq:
        {
            InfoQuery iq = InfoQuery(m_xmlReader);
            token = m_xmlReader.tokenType();
            name = m_xmlReader.name().toUtf8();
            qDebug() << "Received";
            qDebug() << "IQ:" << iq.str();

            QByteArray id = iq.getID();

            switch(m_umapIQWaiting.at(id)){
            case IQPurpose::FEATURE:
                trigger |= ReaderTrigger::FEATURE;
                break;
            case IQPurpose::ROSTER:
            case IQPurpose::DISCO:
                trigger |= ReaderTrigger::INFOQUERY;
                break;
            }
            m_umapIQResults.insert(std::make_pair(std::move(id), std::move(iq)));

            if(m_flgActiveFeatures & Feature::Type::MANAGEMENT){
                auto ft_manag = std::static_pointer_cast<FeatureManagement>(m_umapPersistentFeatures.at(Feature::Type::MANAGEMENT));
                ft_manag->incrInbound();
            }
        }
            break;
        case IntFromString::presence:
        {
            Presence presence(m_xmlReader);
            qDebug() << "Presence:" << presence.str();
            jidfull_t jid = presence.getFrom();
            if(!m_umapChatChains.count(jid)){
                ChatChain* chain = new ChatChain(this, jid, this);
                connect(chain, &ChatChain::receivedMessage,
                        this, &Stream::receivedMessage);
                connect(chain, &ChatChain::sendMessage,
                        this, &Stream::sendStanza);
                m_umapChatChains.insert({jid, chain});
            }
            if(presence.getType().isEmpty()){
                m_ptrContactsModel->insertPresence(presence);
            }else{
                m_ptrContactsModel->removePresence(presence);
            }

            if(m_flgActiveFeatures & Feature::Type::MANAGEMENT){
                auto ft_manag = std::static_pointer_cast<FeatureManagement>(m_umapPersistentFeatures.at(Feature::Type::MANAGEMENT));
                ft_manag->incrInbound();
            }
        }
            break;
        case IntFromString::message:
        {
            Message message(m_xmlReader);
            qDebug() << "Message from:" << message.getFrom() << "reads" << message.getBody();
            qDebug() << message.str();
            jidfull_t jid = message.getFrom();
            if(!m_umapChatChains.count(jid)){
                ChatChain* chain = new ChatChain(this, jid, this);
                connect(chain, &ChatChain::receivedMessage,
                        this, &Stream::receivedMessage);
                connect(chain, &ChatChain::sendMessage,
                        this, &Stream::sendStanza);
                m_umapChatChains.insert({jid, chain});
            }
            m_umapChatChains.at(jid)->addMessage(std::move(message));

            if(m_flgActiveFeatures & Feature::Type::MANAGEMENT){
                auto ft_manag = std::static_pointer_cast<FeatureManagement>(m_umapPersistentFeatures.at(Feature::Type::MANAGEMENT));
                ft_manag->incrInbound();
            }
        }
            break;
        case IntFromString::enabled:
        {
            auto ft_manag = std::static_pointer_cast<FeatureManagement>(m_umapEpheralFeatures.at(Feature::Type::MANAGEMENT));
            ft_manag->state = FeatureManagement::State::ENABLED;
            trigger |= ReaderTrigger::FEATURE;
        }
            break;
        case IntFromString::r:
            if(m_flgActiveFeatures & Feature::Type::MANAGEMENT){
                auto ft_manag = std::static_pointer_cast<FeatureManagement>(m_umapPersistentFeatures.at(Feature::Type::MANAGEMENT));
                QByteArray xmlstrACK = getManagementAck(ft_manag->inbound);
                m_ptrSocket->write(xmlstrACK + "\r\n");
            }
            break;
        case IntFromString::a:
            if(m_flgActiveFeatures & Feature::Type::MANAGEMENT){
                auto ft_manag = std::static_pointer_cast<FeatureManagement>(m_umapPersistentFeatures.at(Feature::Type::MANAGEMENT));
                quint32 counter = m_xmlReader.attributes().value("h").toUInt();
                if(counter == ft_manag->outbound){
                    ft_manag->ack_wait = false;
                }
            }
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case IntFromString::stream:
                finishDisconnect();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateFEATURES(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case IntFromString::starttls:
            m_umapEpheralFeatures.insert({Feature::Type::STARTTLS, std::make_shared<FeatureSTARTTLS>()});
            trigger |= ReaderTrigger::FEATURE;
            m_xmlReaderState = ReaderState::STARTTLS;
            break;
        case IntFromString::mechanisms:
            m_umapEpheralFeatures.insert({Feature::Type::SASL, std::make_shared<FeatureSASL>()});
            trigger |= ReaderTrigger::FEATURE;
            m_xmlReaderState = ReaderState::SASL;
            break;
        case IntFromString::bind:
            m_umapEpheralFeatures.insert({Feature::Type::RESOURCEBIND, std::make_shared<FeatureBind>()});
            trigger |= ReaderTrigger::FEATURE;
            m_xmlReaderState = ReaderState::RESOURCEBIND;
            break;
        case IntFromString::sm:
            if(m_xmlReader.namespaceUri().toString() ==  "urn:xmpp:sm:3"){
                m_umapEpheralFeatures.insert({Feature::Type::MANAGEMENT, std::make_shared<FeatureManagement>()});
                trigger |= ReaderTrigger::FEATURE;
            }
            break;
        default:
            break;
        }
        break;

    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case IntFromString::features:
            m_xmlReaderState = ReaderState::STREAM;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateSTARTTLS(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case IntFromString::required:
            m_umapEpheralFeatures.at(Feature::Type::STARTTLS)->required = true;
            break;
        case IntFromString::proceed:
            std::static_pointer_cast<FeatureSTARTTLS>(m_umapEpheralFeatures.at(Feature::Type::STARTTLS))->state = FeatureSTARTTLS::State::PROCEED;
            trigger |= ReaderTrigger::FEATURE;
            m_xmlReaderState = ReaderState::STREAM;
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case IntFromString::starttls:
            m_xmlReaderState = ReaderState::FEATURES;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateSASL(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case IntFromString::mechanism:
        {
            token = m_xmlReader.readNext();
            name = m_xmlReader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(m_umapEpheralFeatures.at(Feature::Type::SASL));
            feature_sasl->srv_mechanisms.insert(m_xmlReader.text().toString());
            trigger |= ReaderTrigger::FEATURE;
        }
            break;
        case IntFromString::challenge:
        {
            token = m_xmlReader.readNext();
            name = m_xmlReader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(m_umapEpheralFeatures.at(Feature::Type::SASL));
            feature_sasl->challenge = QByteArray::fromBase64(m_xmlReader.text().toUtf8());
            feature_sasl->state = FeatureSASL::State::CHALLENGE;
            trigger |= ReaderTrigger::FEATURE;
            m_xmlReaderState = ReaderState::STREAM;
        }
            break;
        case IntFromString::success:
        {
            token = m_xmlReader.readNext();
            name = m_xmlReader.name().toUtf8();
            std::shared_ptr<FeatureSASL> feature_sasl = std::static_pointer_cast<FeatureSASL>(m_umapEpheralFeatures.at(Feature::Type::SASL));
            feature_sasl->server_sig = QByteArray::fromBase64(m_xmlReader.text().toUtf8());
            feature_sasl->state = FeatureSASL::State::SUCCESS;
            trigger |= ReaderTrigger::FEATURE;
            m_xmlReaderState = ReaderState::STREAM;
        }
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case IntFromString::mechanisms:
            m_xmlReaderState = ReaderState::FEATURES;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Stream::stateRESOURCEBIND(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger){
    switch(token){
    case QXmlStreamReader::StartElement:
        switch(word2int(name)){
        case IntFromString::required:
            m_umapEpheralFeatures.at(Feature::Type::RESOURCEBIND)->required = true;
            trigger |= ReaderTrigger::FEATURE;
            break;
        default:
            break;
        }
        break;
    case QXmlStreamReader::EndElement:
        switch(word2int(name)){
        case IntFromString::bind:
            m_xmlReaderState = ReaderState::FEATURES;
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
    QFlags<ReaderTrigger> triggers;

    while(!m_xmlReader.atEnd()){
        QXmlStreamReader::TokenType token = m_xmlReader.readNext();
        QByteArray name = m_xmlReader.name().toUtf8();
        if(token == QXmlStreamReader::Invalid){
            qDebug() << "ERROR: " << m_xmlReader.error() << m_xmlReader.errorString();
            token = m_xmlReader.readNext();
            name = m_xmlReader.name().toUtf8();
        }
        switch(m_xmlReaderState){

        case ReaderState::INIT:
            stateINIT(token,name,triggers);
            break;
        case ReaderState::STREAM:
            stateSTREAM(token,name,triggers);
            break;
        case ReaderState::FEATURES:
            stateFEATURES(token,name,triggers);
            break;
        case ReaderState::STARTTLS:
            stateSTARTTLS(token,name,triggers);
            break;
        case ReaderState::SASL:
            stateSASL(token,name,triggers);
            break;
        case ReaderState::RESOURCEBIND:
            stateRESOURCEBIND(token,name,triggers);
            break;
        }
        qDebug() << "TOKEN: " << token << m_xmlReader.text() << name << m_xmlReader.namespaceUri();
    }
    if(triggers.testFlag(ReaderTrigger::FEATURE)){
        processFeatures();
    }
    if(triggers.testFlag(ReaderTrigger::INFOQUERY)){
        processInfoQuery();
    }
}

void Stream::processFeatures(){
    bool erase_feature;
    for(auto it = m_umapEpheralFeatures.cbegin(); it != m_umapEpheralFeatures.cend();){
        erase_feature = false;
        switch(it->first){
        case Feature::Type::STARTTLS:
        {
            qDebug() << "Feature STARTTLS | Required: " << it->second->required;
            auto ft_starttls = std::static_pointer_cast<FeatureSTARTTLS>(it->second);
            switch(ft_starttls->state){
            case FeatureSTARTTLS::State::ASK:
                m_ptrSocket->write(getSTARTTLS() + "\r\n");
                m_xmlReaderState = ReaderState::STARTTLS;
                break;
            case FeatureSTARTTLS::State::PROCEED:
                m_xmlReader.clear();
                m_xmlReader.setDevice(m_ptrSocket);
                reconnectSecure();
                m_flgActiveFeatures |= Feature::Type::STARTTLS;
                erase_feature  = true;
                m_xmlReaderState = ReaderState::INIT;
                break;
            case FeatureSTARTTLS::State::FAILURE:
                m_ptrSocket->disconnectFromHost();
                break;
            }
        }
            break;
        case Feature::Type::SASL:
        {
            qDebug() << "Feature SASL | Required: " << it->second->required;
            auto ft_sasl = std::static_pointer_cast<FeatureSASL>(it->second);
            switch(ft_sasl->state){
            case FeatureSASL::State::AUTH:
                for(const auto& sasl : m_qvecSASL){
                    if(ft_sasl->srv_mechanisms.contains(SASL2StrMapper.value(sasl))){
                        ft_sasl->mechanism = sasl;
                        m_SCRAMGenerator = std::make_shared<SASL::SCRAMGenerator>(sasl, m_uptrAccount->credentials());
                        m_SCRAMGenerator->setSCRAMFlag(SASL::SCRAMGenerator::CHANNEL_FLAG::N);
                        m_SCRAMGenerator->writeAttribute(SASL::SCRAMGenerator::Attributes::NAME, m_uptrAccount->jid().local);
                        m_SCRAMGenerator->writeAttribute(SASL::SCRAMGenerator::Attributes::NONCE_CLIENT, Utils::randomString(NONCE_INIT_LEN));
                        m_ptrSocket->write(getAuth() + "\r\n");
                        m_xmlReaderState = ReaderState::SASL;
                        break;
                    }
                }
                // TODO no SASL found
                break;
            case FeatureSASL::State::CHALLENGE:
            {
                m_SCRAMGenerator->loadChallenge(ft_sasl->challenge);
                m_ptrSocket->write(getResponse() + "\r\n");
                m_xmlReaderState = ReaderState::SASL;
            }
                break;
            case FeatureSASL::State::SUCCESS:
            {
                if(!m_SCRAMGenerator->isServerSigValid(ft_sasl->server_sig)){
                    qDebug() << "Invalid server signature";
                    // TODO invalid server signature
                }
                m_xmlReader.clear();
                m_xmlReader.setDevice(m_ptrSocket);
                m_ptrSocket->write(getHeader() + "\r\n");
                m_flgActiveFeatures |= Feature::Type::SASL;
                erase_feature  = true;
                m_xmlReaderState = ReaderState::INIT;
            }
                break;
            }

        }
            break;
        case Feature::Type::RESOURCEBIND:
        {
            qDebug() << "Feature RESOURCEBIND | Required: " << it->second->required;
            auto ft_bind = std::static_pointer_cast<FeatureBind>(it->second);
            if(ft_bind->query_id.isEmpty()){
                InfoQuery iq = InfoQuery();
                iq.generateID();
                iq.setType("set");

                QByteArray id = iq.getID();
                ft_bind->query_id = id;
                m_umapIQWaiting.insert({id, IQPurpose::FEATURE});

                QDomElement bind = iq.createElement("bind");
                bind.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");

                iq.insertNode(bind);

                sendStanza(iq);
            }else{
                InfoQuery iq_result = m_umapIQResults.at(ft_bind->query_id);
                m_umapIQWaiting.erase(m_umapIQWaiting.find(ft_bind->query_id));
                m_umapIQResults.erase(m_umapIQResults.find(ft_bind->query_id));

                switch(word2int(iq_result.getType())){
                case IntFromString::result:
                {
                    QDomNode bind_node = iq_result.root().firstChild();
                    QDomElement jid_elem = bind_node.firstChildElement();

                    jidfull_t jid = jid_elem.text().toUtf8();
                    m_uptrAccount->setJid(jid);
                    m_flgActiveFeatures |= Feature::Type::RESOURCEBIND;
                    erase_feature = true;
                    emit coreEstablished();
                }
                    break;
                case IntFromString::error:
                    // TODO Handle bind errors
                    break;
                default:
                    break;
                }
            }
        }
            break;
        case Feature::Type::MANAGEMENT:
            if(!m_isCoreEstablished){
                break;
            }
        {
            qDebug() << "Feature STREAM MANAGEMENT | Required: " << it->second->required;
            auto ft_manag = std::static_pointer_cast<FeatureManagement>(it->second);
            switch(ft_manag->state){
            case FeatureManagement::State::ASK:
            {
                QByteArray enable = getManagementEnable();
                m_ptrSocket->write(enable + "\r\n");
            }
            break;
            case FeatureManagement::State::ENABLED:
            {
                m_umapPersistentFeatures.insert({Feature::Type::MANAGEMENT, ft_manag});
                m_flgActiveFeatures |= Feature::Type::MANAGEMENT;
                erase_feature = true;
            }
                break;
            }
        }
            break;
        case Feature::Type::UNKNOWN:
            qDebug() << "Feature UNKNOWN | Required: " << it->second->required;

            break;
        }
        if(erase_feature){
            it = m_umapEpheralFeatures.erase(it);
        }else{
            ++it;
        }
    }
}

void Stream::processInfoQuery(){
    auto it = m_umapIQResults.cbegin();
    while(it != m_umapIQResults.cend()){
        QByteArray id = it->first;
        InfoQuery iq = it->second;
        IQPurpose purpose = m_umapIQWaiting.at(id);

        switch(purpose){
        case IQPurpose::ROSTER:
        {
            QDomNode query_node = iq.root().firstChild();
            QDomNodeList item_list = query_node.childNodes();
            for(int i=0; i<item_list.length(); i++){
                QDomElement curr_item = item_list.at(i).toElement();
                rosteritem_t roster(curr_item);
                m_ptrContactsModel->setRoster(roster);
                if(!m_umapChatChains.count(roster.jid)){
                    ChatChain* chain = new ChatChain(this, roster.jid, this);
                    connect(chain, &ChatChain::receivedMessage,
                            this, &Stream::receivedMessage);
                    connect(chain, &ChatChain::sendMessage,
                            this, &Stream::sendStanza);
                    m_umapChatChains.insert({roster.jid, chain});
                }
            }
        }
            break;
        case IQPurpose::DISCO:
        {
            QDomNode query_node = iq.root().firstChild();
            QDomNodeList item_list = query_node.childNodes();
            for(int i=0; i<item_list.length(); i++){
                QDomElement curr_item = item_list.at(i).toElement();
                if(curr_item.nodeName() == "identity"){
                    discoidentity_t identity(curr_item);
                }
                if(curr_item.nodeName() == "feature"){
                    discofeature_t feature(curr_item);
                }
            }
        }
            break;
        default:
            break;
        }
        m_umapIQWaiting.erase(m_umapIQWaiting.find(id));
        it = m_umapIQResults.erase(it);
    }
}

void Stream::queryRoster(){
    InfoQuery iq;
    iq.setFrom(m_uptrAccount->jid().toByteArray());
    iq.generateID();
    iq.setType("get");

    m_umapIQWaiting.insert({iq.getID(), IQPurpose::ROSTER});

    QDomElement query = iq.createElement("query");
    query.setAttribute("xmlns", "jabber:iq:roster");

    iq.insertNode(query);

    sendStanza(iq);
}

void Stream::queryDisco(){
    InfoQuery iq;
    iq.setFrom(m_uptrAccount->jid().toByteArray());
    iq.generateID();
    iq.setType("get");

    m_umapIQWaiting.insert({iq.getID(), IQPurpose::DISCO});

    QDomElement query = iq.createElement("query");
    query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");

    iq.insertNode(query);

    sendStanza(iq);
}

void Stream::advertisePresence(bool status){
    Presence presence;
    if(!status){
        presence.setType("unavailable");
    }
    sendStanza(presence);
}

void Stream::initDisconnect(){
    m_ptrSocket->write(getHeaderEnd() + "\r\n");
    m_isConnClosing = true;
}

void Stream::finishDisconnect(){
    if(!m_isConnClosing){
        m_ptrSocket->write(getHeaderEnd() + "\r\n");
    }
    m_ptrSocket->disconnectFromHost();
    emit disconnected();
}

void Stream::sendStanza(const Stanza& stanza){
    qDebug() << "Sending";
    switch(stanza.type()){
    case Stanza::Type::INFOQUERY:
    {
        auto iq = static_cast<const InfoQuery*>(&stanza);
        qDebug() << "IQ to:" << iq->getTo() << "from" << iq->getFrom();
    }
        break;
    case Stanza::Type::PRESENCE:
    {
        auto presence = static_cast<const Presence*>(&stanza);
        qDebug() << "Presence to:" << presence->getTo() << "from" << presence->getFrom();
    }
        break;
    case Stanza::Type::MESSAGE:
    {
        auto msg = static_cast<const Message*>(&stanza);
        qDebug() << "Message to:" << msg->getTo() << "reads" << msg->getBody();
    }
        break;
    }
    qDebug() << stanza.str();
    m_ptrSocket->write(stanza.str() + "\r\n");

    if(m_flgActiveFeatures & Feature::Type::MANAGEMENT){
        auto manager = std::static_pointer_cast<FeatureManagement>(m_umapPersistentFeatures.at(Feature::Type::MANAGEMENT));
        manager->incrOutbound();
        manager->ack_wait = true;
        m_ptrSocket->write(getManagementReq() + "\r\n");
    }
}

void Stream::socketError(QAbstractSocket::SocketError){
    qDebug() << "Socket error:" << m_ptrSocket->errorString();
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
    writer.writeAttribute("from",     m_uptrAccount->jid().toByteArray());
    writer.writeAttribute("to",       m_uptrServer->jid().domain);
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
    writer.writeAttribute("mechanism", SASL2StrMapper.value(m_SCRAMGenerator->GetSASL()));
    writer.writeCharacters(m_SCRAMGenerator->getInitResponse().toBase64());
    writer.writeEndElement();
    return arr;
}

QByteArray Stream::getResponse(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("response");
    writer.writeAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    writer.writeCharacters(m_SCRAMGenerator->getChallengeResponse().toBase64());
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

QByteArray Stream::getManagementEnable(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("enable");
    writer.writeAttribute("xmlns", "urn:xmpp:sm:3");
    writer.writeEndElement();
    return arr;
}

QByteArray Stream::getManagementReq(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("r");
    writer.writeAttribute("xmlns", "urn:xmpp:sm:3");
    writer.writeEndElement();
    return arr;
}

QByteArray Stream::getManagementAck(quint32 counter){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("a");
    writer.writeAttribute("xmlns", "urn:xmpp:sm:3");
    writer.writeAttribute("h", QByteArray::number(counter));
    writer.writeEndElement();
    return arr;
}
