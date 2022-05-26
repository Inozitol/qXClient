#ifndef INFOQUERY_H
#define INFOQUERY_H

#include "stanza.h"

class InfoQuery : public Stanza
{
public:
    InfoQuery();
    InfoQuery(QXmlStreamReader& reader);
};

#endif // INFOQUERY_H
