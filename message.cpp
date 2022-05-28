#include "message.h"

Message::Message()
    : Stanza("message")
{}

Message::Message(QXmlStreamReader& reader)
    : Stanza("message")
{
    if(reader.tokenType() != QXmlStreamReader::StartElement){
        throw std::invalid_argument("Message expects reader on element start");
    }
    if(reader.name().toUtf8() != "message"){
        throw std::invalid_argument("Message expects reader on message stanza");
    }

    for(const auto& attr : reader.attributes()){
        switch(word2int(attr.name().toUtf8())){
        case XMLWord::to:
            setTo(attr.value().toUtf8());
            break;
        case XMLWord::from:
            setFrom(attr.value().toUtf8());
            break;
        case XMLWord::id:
            setId(attr.value().toUtf8());
            break;
        case XMLWord::type:
            setType(attr.value().toUtf8());
            break;
        case XMLWord::lang:
            setLang(attr.value().toUtf8());
            break;
        default:
            break;
        }
    }

    Utils::reader2node(*this, _stanza, reader);

    QDomNodeList msg_child_list = root().childNodes();
    for(int i=0; i<msg_child_list.length(); i++){
        QDomElement curr_item = msg_child_list.at(i).toElement();
        switch(word2int(curr_item.tagName().toUtf8())){
        case XMLWord::body:
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
        case XMLWord::subject:
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
        default:
            break;
        }
    }
}

QString Message::getBody(const QLocale& locale){
    return _body_map.value(locale);
}

QString Message::getBody(){
    return _body_map.value(_def_locale);
}

QString Message::getSubject(const QLocale& locale){
    return _subj_map.value(locale);
}

QString Message::getSubject(){
    return _subj_map.value(_def_locale);
}
