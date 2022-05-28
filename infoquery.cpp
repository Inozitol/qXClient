#include "infoquery.h"

InfoQuery::InfoQuery()
    : Stanza("iq")
{}

InfoQuery::InfoQuery(QXmlStreamReader& reader)
    : Stanza("iq")
{
    if(reader.tokenType() != QXmlStreamReader::StartElement){
        throw std::invalid_argument("InfoQuery expects reader on element start");
    }
    if(reader.name().toUtf8() != "iq"){
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

    Utils::reader2node(*this, _stanza, reader);


}
