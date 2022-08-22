#include "message.h"

Message::Message()
    : Stanza(Stanza::Type::MESSAGE)
{}

Message::Message(const Message& message)
    : Stanza(message){
    _body_map = message._body_map;
    _subj_map = message._subj_map;
}

Message::Message(Message&& message)
    : Stanza(std::move(message)){
    _body_map = message._body_map;
    _subj_map = message._subj_map;

    message._body_map.clear();
    message._subj_map.clear();
}

Message::Message(QXmlStreamReader& reader)
    : Stanza(Stanza::Type::MESSAGE)
{
    if(reader.tokenType() != QXmlStreamReader::StartElement){
        throw std::invalid_argument("Message expects reader on element start");
    }
    if(reader.name().toUtf8() != "message"){
        throw std::invalid_argument("Message expects reader on message stanza");
    }

    for(const auto& attr : reader.attributes()){
        switch(word2int(attr.name().toUtf8())){
        case IntFromString::to:
            setTo(attr.value().toUtf8());
            break;
        case IntFromString::from:
            setFrom(attr.value().toUtf8());
            break;
        case IntFromString::id:
            setId(attr.value().toUtf8());
            break;
        case IntFromString::type:
            setType(attr.value().toUtf8());
            break;
        case IntFromString::lang:
            setLang(attr.value().toUtf8());
            break;
        default:
            break;
        }
    }
    if(getType().isEmpty()){
        setType("normal");
    }

    Utils::reader2node(*this, _stanza, reader);

    QDomNodeList msg_child_list = root().childNodes();
    for(int i=0; i<msg_child_list.length(); i++){
        QDomElement curr_item = msg_child_list.at(i).toElement();
        switch(word2int(curr_item.tagName().toUtf8())){
        case IntFromString::body:
        {
            QString text = curr_item.text().toUtf8();
            QByteArray lang = curr_item.attribute("lang").toUtf8();
            if(!lang.isEmpty()){
                _body_map.insert(QLocale(lang), text);
            }else{
                _body_map.insert(_def_locale, text);
            }
        }
            break;
        case IntFromString::subject:
        {
            QString text = curr_item.text();
            QByteArray lang = curr_item.attribute("lang").toUtf8();
            if(!lang.isEmpty()){
                _subj_map.insert(QLocale(lang), text);
            }else{
                _subj_map.insert(_def_locale, text);
            }
        }
            break;
        case IntFromString::thread:
        {
            QString thread = curr_item.text();
            setThread(thread.toUtf8());
            if(curr_item.hasAttribute("parent")){
                QString parent = curr_item.attribute("parent");
                setThreadParent(parent.toUtf8());
            }
        }
            break;
        case IntFromString::delay:
        {
            QString strTimestamp = curr_item.attribute("stamp");
            _timestamp = QDateTime::fromString(strTimestamp, Qt::ISODate);
            m_flgFlags |= Flag::DELAYED;
        }
            break;
        default:
            break;
        }
    }
}

QString Message::getBody(const QLocale& locale) const{
    return _body_map.value(locale);
}

QString Message::getBody() const{
    return _body_map.value(_def_locale);
}

void Message::setBody(const QString& str, const QLocale& locale){
    QDomElement bodyElem = createElement("body");
    QDomText textNode = createTextNode(str);
    bodyElem.appendChild(textNode);
    _stanza.appendChild(bodyElem);

    _body_map[locale] = str;
}

void Message::setBody(const QString& str){
    QDomElement bodyElem = createElement("body");
    QDomText textNode = createTextNode(str);
    bodyElem.appendChild(textNode);
    _stanza.appendChild(bodyElem);

    _body_map[_def_locale] = str;
}

QString Message::getSubject() const{
    return _subj_map.value(_def_locale);
}


QString Message::getSubject(const QLocale& locale) const{
    return _subj_map.value(locale);
}

void Message::setThread(const QByteArray& thread){
    _thread = thread;

    QDomElement bodyElem = createElement("thread");
    QDomText textNode = createTextNode(thread);
    bodyElem.appendChild(textNode);
    _stanza.appendChild(bodyElem);

    _threadElem = bodyElem;
}

void Message::setThread(QByteArray &&thread){
    _thread = thread;

    QDomElement bodyElem = createElement("thread");
    QDomText textNode = createTextNode(std::move(thread));
    bodyElem.appendChild(textNode);
    _stanza.appendChild(bodyElem);

    _threadElem = bodyElem;
}

void Message::setThread(){
    QByteArray thread = Utils::randomString(THREAD_LEN);

    QDomElement bodyElem = createElement("thread");
    QDomText textNode = createTextNode(thread);
    bodyElem.appendChild(textNode);
    _stanza.appendChild(bodyElem);

    _thread = thread;
    _threadElem = bodyElem;
}

void Message::setThreadParent(const QByteArray& thread){
    if(_thread.isEmpty()){
        setThread();
    }
    _threadElem.setAttribute("parent", thread);
}

void Message::setThreadParent(QByteArray&& thread){
    if(_thread.isEmpty()){
        setThread();
    }
    _threadElem.setAttribute("parent", std::move(thread));
}

void Message::setThreadParent(){
    QByteArray thread = Utils::randomString(THREAD_LEN);

    if(_thread.isEmpty()){
        setThread();
    }
    _threadElem.setAttribute("parent", std::move(thread));
}

void Message::setFlag(Flag flgs){
    m_flgFlags |= flgs;
}

bool Message::isDelayed(){
    return m_flgFlags.testFlag(Flag::DELAYED);
}

QString Message::payloadNS() const{
    return _stanza.firstChildElement().namespaceURI();
}
