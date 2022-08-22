#ifndef INFOQUERY_H
#define INFOQUERY_H

#include "stanza.h"

class InfoQuery : public Stanza
{
public:
    InfoQuery();
    InfoQuery(const InfoQuery& iq);
    InfoQuery(InfoQuery&& iq);
    InfoQuery(QXmlStreamReader& reader);

    InfoQuery& operator=(const InfoQuery&)=default;

    QString payloadNS() const;
};

#endif // INFOQUERY_H
