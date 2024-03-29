#include "infoquery.h"

InfoQuery::InfoQuery()
    : Stanza(Stanza::Type::INFOQUERY)
{}

InfoQuery::InfoQuery(const InfoQuery &iq)
    : Stanza(iq)
{}

InfoQuery::InfoQuery(InfoQuery &&iq)
    : Stanza(std::move(iq))
{}

InfoQuery::InfoQuery(QXmlStreamReader& reader)
    : Stanza(Stanza::Type::INFOQUERY)
{
    if(reader.tokenType() != QXmlStreamReader::StartElement){
        throw std::invalid_argument("InfoQuery expects reader on element start");
    }
    if(reader.name().toUtf8() != "iq"){
        throw std::invalid_argument("InfoQuery expects reader on InfoQuery stanza");
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
}

QString InfoQuery::payloadNS() const{
    return _stanza.firstChildElement().namespaceURI();
}
