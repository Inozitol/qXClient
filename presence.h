#ifndef PRESENCE_H
#define PRESENCE_H

#include <QByteArray>
#include <QtXml>

#include "addressable.h"
#include "strswitch.h"

struct presence_t{
    jid_t from;
    jid_t to;
    QByteArray id;
    QByteArray type;
    QByteArray show;
    QMap<QByteArray, QByteArray> status_map;

    presence_t(QXmlStreamReader& reader){
        if(reader.tokenType() != QXmlStreamReader::StartElement){
            throw std::invalid_argument("presence_t expects reader on element start");
        }
        if(reader.name().toUtf8() != "presence"){
            throw std::invalid_argument("presence_t expects reader on presence tag");
        }

        QByteArray def_lang;

        for(const auto& attr : reader.attributes()){
            switch(word2int(attr.name().toUtf8())){
            case XMLWord::from:
                from = attr.value().toUtf8();
                break;
            case XMLWord::to:
                to = attr.value().toUtf8();
                break;
            case XMLWord::id:
                id = attr.value().toUtf8();
                break;
            case XMLWord::type:
                type = attr.value().toUtf8();
                break;
            case XMLWord::lang:
                def_lang = attr.value().toUtf8();
                break;
            default:
                break;
            }
        }

        def_lang = (def_lang.isEmpty()) ? "en" : def_lang;

        QXmlStreamReader::TokenType token = reader.readNext();
        QByteArray name = reader.name().toUtf8();
        while(name != "presence"){
            switch(token){
            case QXmlStreamReader::StartElement:
            {
                switch(word2int(name)){
                case XMLWord::show:
                    reader.readNext();
                    show = reader.text().toUtf8();
                    break;

                case XMLWord::status:
                {
                    QByteArray lang = reader.attributes().value("lang").toUtf8();
                    lang = (lang.isEmpty()) ? def_lang : lang;
                    reader.readNext();
                    status_map[lang] = reader.text().toUtf8();
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
            token = reader.readNext();
            name = reader.name().toUtf8();
        }
    }

    presence_t() = default;
};

#endif // PRESENCE_H
