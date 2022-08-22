#include "stream.h"

Stream::Stream(const Account& account, const Server& server)
    : m_sptrAccount(std::make_shared<Account>(account)),
      m_sptrServer(std::make_shared<Server>(server)),
      m_disco(m_sptrAccount,m_sptrServer, this),
      m_pubsub(m_sptrAccount,m_sptrServer, this)
{
    initSocket();
    initBuffer();
    initSignals();
    m_xmlReader.setDevice(m_ptrBuffer);
}

Stream::Stream(Account&& account, Server&& server)
    : m_sptrAccount(std::make_shared<Account>(std::move(account))),
      m_sptrServer(std::make_shared<Server>(std::move(server))),
      m_disco(m_sptrAccount,m_sptrServer, this),
      m_pubsub(m_sptrAccount,m_sptrServer, this)
{
    initSocket();
    initBuffer();
    initSignals();
    m_xmlReader.setDevice(m_ptrBuffer);
}

Stream::~Stream(){
    delete(m_ptrSocket);
    delete(m_ptrBuffer);
    delete(m_ptrContactsModel);
}

jidbare_t Stream::accountJid(){
    return m_sptrAccount->jid();
}

ContactTreeModel* Stream::contactsModel(){
    return m_ptrContactsModel;
}

ChatChain* Stream::chatChain(const jidbare_t& jid){
    if(m_umapChatChains.count(jid)){
        return m_umapChatChains.at(jid);
    }
    return nullptr;
}

void Stream::connectInsecure(){
    m_ptrSocket->connectToHost(m_sptrServer->jid().domain, m_sptrServer->getPort());
    m_ptrSocket->write(getHeader() + "\r\n");
}

void Stream::initSocket(){
    if(m_ptrSocket) return;

    m_ptrSocket = new QSslSocket(this);

    connect(m_ptrSocket, &QSslSocket::stateChanged,  this, [](QAbstractSocket::SocketState state){ qDebug() << "Socket state change:" << state;});
    connect(m_ptrSocket, &QSslSocket::readyRead,     this, &Stream::readData);
    connect(m_ptrSocket, &QSslSocket::errorOccurred, this, &Stream::socketError);
    connect(m_ptrSocket, &QSslSocket::disconnected,  this, &Stream::initDisconnect);
    connect(m_ptrSocket, &QSslSocket::encrypted,     this, [](){qDebug() << "SSL Encrypted";});
    connect(m_ptrSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &Stream::sslErrors);
}

void Stream::initBuffer(){
    if(m_ptrBuffer) return;

    m_ptrBuffer = new QBuffer(this);
    m_ptrBuffer->open(QIODevice::ReadWrite);
}

void Stream::initSignals(){
    connect(this, &Stream::coreEstablished, this, [this](){ m_isCoreEstablished = true; });
    connect(this, &Stream::coreEstablished, this, [this](){ m_ptrContactsModel = new ContactTreeModel(m_sptrAccount->jid(), this); });
    connect(this, &Stream::coreEstablished, this, &Stream::queryRoster);
    connect(this, &Stream::coreEstablished, this, [this](){ m_disco.initQuery(); });
    connect(this, &Stream::coreEstablished, this, [this](){ advertisePresence(true); });

    connect(&m_disco, &Disco::sendInfoQuery, this, &Stream::sendStanza);
    connect(&m_disco, &Disco::gotFeature,    this, &Stream::implementDiscoFeature);

    connect(&m_disco, &Disco::pubsubNodeDiscovered, &m_pubsub, &PubSub::pubsubNodeDiscovered);

    connect(&m_pubsub, &PubSub::sendInfoQuery, this, &Stream::sendStanza);
    connect(&m_pubsub, &PubSub::avatarUpdated, this, &Stream::updateAvatar);
}

void Stream::reconnectSecure(){
    m_ptrSocket->startClientEncryption();
    m_ptrSocket->write(getHeader() + "\r\n");
}

void Stream::processData(){
    QFlags<ReaderTrigger> triggers;

    while(!m_xmlReader.atEnd()){
        QXmlStreamReader::TokenType token = m_xmlReader.readNext();
        QByteArray name = m_xmlReader.name().toUtf8();
        if(token == QXmlStreamReader::Invalid){
            qDebug() << "XML reader error token type ["
                     << m_xmlReader.error()
                     << "] reads ["
                     << m_xmlReader.errorString()
                     << "]";

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
        qDebug() << "XML reader token type ["
                 << token
                 << "] | text ["
                 << m_xmlReader.text()
                 << "] | name ["
                 << name
                 << "] | namespace ["
                 << m_xmlReader.namespaceUri()
                 << "]";
    }
    if(triggers.testFlag(ReaderTrigger::FEATURE)){
        processFeatures();
    }
    if(triggers.testFlag(ReaderTrigger::INFOQUERY)){
        processInfoQuery();
    }
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
            if(iq.getFrom().isEmpty()){
                iq.setFrom(m_sptrServer->jid().bare().toByteArray());
            }
            token = m_xmlReader.tokenType();
            name = m_xmlReader.name().toUtf8();

            qInfo() << "Received stanza IQ | from ["
                    << iq.getFrom()
                    << "] | ID ["
                    << iq.getID()
                    << "] | NS ["
                    << iq.payloadNS().toUtf8()
                    << "]";

            qDebug() << "Received stanza IQ |"
                     << iq.str();

            QByteArray ns = iq.payloadNS().toUtf8();

            switch(word2int(ns)){
            case IntFromString::ns_xml_xmpp_bind:
                trigger |= ReaderTrigger::FEATURE;
                break;

            case IntFromString::jabber_iq_roster:
            case IntFromString::http_jabber_disco_info:
            case IntFromString::http_jabber_disco_items:
            case IntFromString::http_jabber_pubsub:
                trigger |= ReaderTrigger::INFOQUERY;
            default:
                break;
            }

            jidfull_t fromJid = iq.getFrom();
            QByteArray id = iq.getID();

            if(m_umapIQResults.count(fromJid)){
                m_umapIQResults.at(fromJid).insert({std::move(id),std::move(iq)});
            }else{
                std::unordered_map<QByteArray, InfoQuery> tmpMap;
                tmpMap.insert({std::move(id),std::move(iq)});
                m_umapIQResults.insert({std::move(fromJid),std::move(tmpMap)});
            }

            if(m_flgActiveFeatures & Feature::Type::MANAGEMENT){
                auto ft_manag = std::static_pointer_cast<FeatureManagement>(m_umapPersistentFeatures.at(Feature::Type::MANAGEMENT));
                ft_manag->incrInbound();
            }
        }
            break;
        case IntFromString::presence:
        {
            Presence presence(m_xmlReader);
            qInfo() << "Received stanza Presence | from ["
                    << presence.getFrom()
                    << "] | ID ["
                    << presence.getID()
                    << "]";

            qDebug() << "Received stanza Presence |"
                     << presence.str();

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
            if(jid.bare() != m_sptrAccount->jid().bare()){
                m_disco.queryPresence(presence);
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
            qInfo() << "Received stanza Message | from ["
                    << message.getFrom()
                    << "] | ID ["
                    << message.getID()
                    << "] | Type ["
                    << message.getType()
                    << "]";

            qDebug() << "Received stanza Message |"
                     << message.str();

            switch(word2int(message.getType())){
                case IntFromString::chat:
                {
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
                }
                break;
                case IntFromString::headline:
                {
                QByteArray ns = message.payloadNS().toUtf8();
                switch(word2int(ns)){
                    case IntFromString::http_jabber_pubsub_event:
                    {
                        m_pubsub.processEventMessage(message);
                    }
                    break;

                default:
                    break;
                }

                }
                break;
            default:
                break;
            }


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
        case IntFromString::c:
            m_umapEpheralFeatures.insert({Feature::Type::CAPS, std::make_shared<FeatureCaps>()});
            trigger |= ReaderTrigger::FEATURE;
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
    static bool waiting;    // default false
    static QByteArray dataBuffer;
    qint64 dataSize = 0;
    QDomDocument testingDoc;
    if(!waiting){
        m_ptrBuffer->buffer().clear();
    }
    m_ptrBuffer->seek(0);
    while(m_ptrSocket->bytesAvailable() > 0){
        QByteArray tmpData = m_ptrSocket->readAll();
        dataBuffer.append(tmpData);
    }
    dataSize = dataBuffer.length();
    QByteArray testingData = "<?xml version='1.0'?><doc>" + dataBuffer + "</doc>";
    bool parsed = testingDoc.setContent(testingData);
    // This is wrong on multiple levels
    // Will fix this later after figuring out what the hell is happening here
    if((!parsed && dataSize == 4096) || (dataBuffer.front() != '<' || dataBuffer.back() != '>')){
        waiting = true;
        return;
    }
    qDebug() << dataBuffer;
    m_ptrBuffer->write(dataBuffer);
    dataBuffer.clear();
    waiting = false;
    m_ptrBuffer->seek(0);
    processData();
}

void Stream::processFeatures(){
    bool erase_feature;
    for(auto it = m_umapEpheralFeatures.cbegin(); it != m_umapEpheralFeatures.cend();){
        erase_feature = false;
        switch(it->first){
        case Feature::Type::STARTTLS:
        {
            qDebug() << "Processing feature STARTTLS | Required:" << it->second->required;
            auto ft_starttls = std::static_pointer_cast<FeatureSTARTTLS>(it->second);
            switch(ft_starttls->state){
            case FeatureSTARTTLS::State::ASK:
                m_ptrSocket->write(getSTARTTLS() + "\r\n");
                m_xmlReaderState = ReaderState::STARTTLS;
                break;
            case FeatureSTARTTLS::State::PROCEED:
                m_xmlReader.clear();
                m_xmlReader.setDevice(m_ptrBuffer);
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
            qDebug() << "Processing feature SASL | Required:" << it->second->required;
            auto ft_sasl = std::static_pointer_cast<FeatureSASL>(it->second);
            switch(ft_sasl->state){
            case FeatureSASL::State::AUTH:
                for(const auto& sasl : m_qvecSASL){
                    if(ft_sasl->srv_mechanisms.contains(SASL2StrMapper.value(sasl))){
                        ft_sasl->mechanism = sasl;
                        switch(sasl){
                        case SASLSupported::SCRAM_SHA_1:
                        {
                            m_SASLGenerator = std::make_shared<SASL::SCRAMGenerator>(sasl, m_sptrAccount->credentials());
                            auto SCRAMGenerator = std::static_pointer_cast<SASL::SCRAMGenerator>(m_SASLGenerator);
                            SCRAMGenerator->setSCRAMFlag(SASL::SCRAMGenerator::CHANNEL_FLAG::N);
                            SCRAMGenerator->writeAttribute(SASL::SCRAMGenerator::Attributes::NAME, m_sptrAccount->jid().local);
                            SCRAMGenerator->writeAttribute(SASL::SCRAMGenerator::Attributes::NONCE_CLIENT, Utils::randomString(NONCE_INIT_LEN));
                            m_ptrSocket->write(getAuth() + "\r\n");
                            m_xmlReaderState = ReaderState::SASL;
                        }
                            break;
                        case SASLSupported::PLAIN:
                        {
                            m_SASLGenerator = std::make_shared<SASL::PLAINGenerator>(sasl, m_sptrAccount->credentials());
                            auto PLAINGenerator = std::static_pointer_cast<SASL::PLAINGenerator>(m_SASLGenerator);
                            PLAINGenerator->writeID(m_sptrAccount->jid().local);
                            m_ptrSocket->write(getAuth() + "\r\n");
                            m_xmlReaderState = ReaderState::SASL;
                        }
                            break;
                        }
                        break;
                    }
                }
                // TODO no SASL found
                break;
            case FeatureSASL::State::CHALLENGE:
            {
                m_SASLGenerator->loadChallenge(ft_sasl->challenge);
                m_ptrSocket->write(getResponse() + "\r\n");
                m_xmlReaderState = ReaderState::SASL;
            }
                break;
            case FeatureSASL::State::SUCCESS:
            {
                auto ft_sasl = std::static_pointer_cast<FeatureSASL>(it->second);
                switch(ft_sasl->mechanism){
                case SASLSupported::SCRAM_SHA_1:
                {
                    std::shared_ptr<SASL::SCRAMGenerator> SCRAMGenerator = std::static_pointer_cast<SASL::SCRAMGenerator>(m_SASLGenerator);
                    if(!SCRAMGenerator->isServerSigValid(ft_sasl->server_sig)){
                        qDebug() << "Invalid server signature";
                        // TODO invalid server signature
                    }
                }
                    break;
                case SASLSupported::PLAIN:
                    break;
                }
                m_xmlReader.clear();
                m_xmlReader.setDevice(m_ptrBuffer);
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
            qDebug() << "Processing feature RESOURCEBIND | Required:" << it->second->required;
            auto ft_bind = std::static_pointer_cast<FeatureBind>(it->second);
            if(ft_bind->query_id.isEmpty()){
                InfoQuery iq = InfoQuery();
                iq.generateID();
                iq.setType("set");

                QByteArray id = iq.getID();
                ft_bind->query_id = id;

                QDomElement bind = iq.createElementNS("urn:ietf:params:xml:ns:xmpp-bind","bind");

                iq.insertNode(bind);

                sendStanza(iq);
            }else{
                InfoQuery iq_result = m_umapIQResults.at(m_sptrServer->jid().domain).at(ft_bind->query_id);

                switch(word2int(iq_result.getType())){
                case IntFromString::result:
                {
                    QDomNode bind_node = iq_result.root().firstChild();
                    QDomElement jid_elem = bind_node.firstChildElement();

                    jidfull_t jid = jid_elem.text().toUtf8();
                    m_sptrAccount->setJid(jid);
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
                m_umapIQResults.at(m_sptrServer->jid().domain).erase(ft_bind->query_id);
                if(m_umapIQResults.at(m_sptrServer->jid().domain).empty()){
                    m_umapIQResults.erase(m_sptrServer->jid().domain);
                }
            }
        }
            break;
        case Feature::Type::MANAGEMENT:
            if(!m_isCoreEstablished){
                break;
            }
        {
            qDebug() << "Processing feature STREAM MANAGEMENT | Required:" << it->second->required;
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
        case Feature::Type::CAPS:
            if(!(m_flgActiveFeatures & Feature::Type::RESOURCEBIND)){
                break;
            }

        {
            qDebug() << "Processing feature CAPS | Required:" << it->second->required;
            auto ft_caps = std::static_pointer_cast<FeatureCaps>(it->second);
            m_umapPersistentFeatures.insert({Feature::Type::CAPS, ft_caps});
            m_flgActiveFeatures |= Feature::Type::CAPS;
            erase_feature = true;

            // TODO Make a check if the stream got its presence advertised or not
            //advertisePresence(true);
        }
            break;
        case Feature::Type::UNKNOWN:
            qDebug() << "Got unknown/unsupported feature | Required:" << it->second->required;

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
    // Load maps identified by jid
    auto it_jidmap = m_umapIQResults.begin();
    while(it_jidmap != m_umapIQResults.end()){

        // Load maps identified by id
        auto it_idmap = it_jidmap->second.cbegin();
        while(it_idmap != it_jidmap->second.cend()){

            InfoQuery iq = it_idmap->second;
            QByteArray ns = iq.payloadNS().toUtf8();

            // Switch by namespace of payload
            switch(word2int(ns)){
            case IntFromString::jabber_iq_roster:
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
            case IntFromString::http_jabber_disco_info:
            case IntFromString::http_jabber_disco_items:
                m_disco.processQuery(iq);
                break;
            case IntFromString::http_jabber_pubsub:
                m_pubsub.processInfoQuery(iq);
                break;
            default:
                break;
            }

            // Erase after processing
            it_idmap = it_jidmap->second.erase(it_idmap);
        }
        // Erase after processing
        it_jidmap = m_umapIQResults.erase(it_jidmap);
    }
}

void Stream::queryRoster(){
    InfoQuery iq;
    iq.setFrom(m_sptrAccount->jid().toByteArray());
    iq.generateID();
    iq.setType("get");

    QDomElement query = iq.createElementNS("jabber:iq:roster","query");

    iq.insertNode(query);

    sendStanza(iq);
}

void Stream::advertisePresence(bool status){
    Presence presence;
    if(!status){
        presence.setType("unavailable");
    }
    auto ft_caps = std::static_pointer_cast<FeatureCaps>(m_umapEpheralFeatures.at(Feature::Type::CAPS));
    QDomElement c = presence.createElementNS("http://jabber.org/protocol/caps","c");
    switch(caps_data::hashAlgo){
    case QCryptographicHash::Sha1:
        c.setAttribute("hash", "sha-1");
        break;
    default:
        break;
    }
    c.setAttribute("node",caps_data::node);
    c.setAttribute("ver",caps_data::getVerString().toBase64());
    presence.insertNode(c);

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

void Stream::implementDiscoFeature(const jidfull_t &jid, Disco::Feature feature){
    switch(feature){
    case Disco::Feature::avatar_metadata_notify:
        //m_pubsub.subscribe(jid, "urn:xmpp:avatar:metadata");
        break;
    default:
        break;
    }

}

void Stream::updateAvatar(const jidfull_t &jid, const QString &id){
    m_ptrContactsModel->updateAvatarId(jid, id);
}

void Stream::sendStanza(const Stanza& stanza){

    switch(stanza.type()){
    case Stanza::Type::INFOQUERY:
    {
        auto iq = static_cast<const InfoQuery*>(&stanza);
        qInfo() << "Sending stanza IQ | from ["
                << iq->getFrom()
                << "] | ID ["
                << iq->getID()
                << "]";

        qDebug() << "Sending stanza IQ |"
                 << stanza.str();
    }
        break;
    case Stanza::Type::PRESENCE:
    {
        auto presence = static_cast<const Presence*>(&stanza);
        qInfo() << "Sending stanza Presence | from ["
                << presence->getFrom()
                << "] | ID ["
                << presence->getID()
                << "]";

        qDebug() << "Sending stanza Presence |"
                 << stanza.str();
    }
        break;
    case Stanza::Type::MESSAGE:
    {
        auto msg = static_cast<const Message*>(&stanza);
        qInfo() << "Sending stanza Message | from ["
                << msg->getFrom()
                << "] | ID ["
                << msg->getID()
                << "]";

        qDebug() << "Sending stanza Message |"
                 << stanza.str();
    }
        break;
    }
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
    writer.writeAttribute("from",     m_sptrAccount->jid().toByteArray());
    writer.writeAttribute("to",       m_sptrServer->jid().domain);
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
    writer.writeAttribute("mechanism", SASL2StrMapper.value(m_SASLGenerator->GetSASL()));
    writer.writeCharacters(m_SASLGenerator->getInitResponse().toBase64());
    writer.writeEndElement();
    return arr;
}

QByteArray Stream::getResponse(){
    QByteArray arr;
    QXmlStreamWriter writer(&arr);
    writer.writeStartElement("response");
    writer.writeAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
    writer.writeCharacters(m_SASLGenerator->getChallengeResponse().toBase64());
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
