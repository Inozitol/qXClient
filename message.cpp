#include "message.h"

Message::Message()
    : Stanza("message")
{}

Message::Message(QXmlStreamReader& reader)
    : Stanza("message")
{
    if(reader.tokenType() != QXmlStreamReader::StartElement){
        throw std::invalid_argument("InfoQuery expects reader on element start");
    }
    if(reader.name().toUtf8() != "message"){
        throw std::invalid_argument("InfoQuery expects reader on InfoQuery stanza");
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

    QXmlStreamReader::TokenType token = reader.readNext();
    QByteArray name = reader.name().toUtf8();
    QDomElement currNode = _stanza.toElement();
    while(name != "message"){
        switch(token){
        case QXmlStreamReader::StartElement:
        {
            QDomElement newNode = createElement(name);
            for(const auto& attr : reader.attributes()){
                newNode.setAttribute(attr.name().toString(), attr.value().toString());
            }
            currNode = currNode.appendChild(newNode).toElement();
        }
            break;
        case QXmlStreamReader::Characters:
        {
            QDomText textNode = createTextNode(reader.text().toString());
            currNode = currNode.appendChild(textNode).toElement();
        }
            break;
        default:
            break;
        }
        token = reader.readNext();
        name = reader.name().toUtf8();
    }
}
