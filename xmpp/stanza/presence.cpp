#include "presence.h"

Presence::Presence()
    :Stanza(Stanza::Type::PRESENCE)
{}

Presence::Presence(QXmlStreamReader& reader)
    :Stanza(Stanza::Type::PRESENCE)
{
    if(reader.tokenType() != QXmlStreamReader::StartElement){
        throw std::invalid_argument("Presence expects reader on element start");
    }
    if(reader.name().toUtf8() != "presence"){
        throw std::invalid_argument("Presence expects reader on Presence stanza");
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

    Utils::reader2node(*this, _stanza, reader);

    QDomNodeList msg_child_list = root().childNodes();
    for(int i=0; i<msg_child_list.length(); i++){
        QDomElement curr_item = msg_child_list.at(i).toElement();
        switch(word2int(curr_item.tagName().toUtf8())){
        case IntFromString::status:
        {
            QString text = curr_item.text().toUtf8();
            QByteArray lang = curr_item.attribute("lang").toUtf8();
            if(!lang.isEmpty()){
                _status_map.insert(QLocale(lang), text);
            }else{
                _status_map.insert(_def_locale, text);
            }
        }
            break;
        default:
            break;
        }
    }
}

QString Presence::getStatus() const{
    return _status_map.value(_def_locale);
}

QString Presence::getStatus(const QLocale& locale) const{
    return _status_map.value(locale);
}
