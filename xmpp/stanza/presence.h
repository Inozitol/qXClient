#ifndef PRESENCE_H
#define PRESENCE_H

#include "stanza.h"

class Presence : public Stanza
{
public:
    Presence();
    Presence(QXmlStreamReader& reader);

    Presence& operator=(const Presence&)=default;

    QString getStatus() const;
    QString getStatus(const QLocale& locale) const;
private:
    QHash<QLocale, QString> _status_map;
};

#endif // PRESENCE_H
