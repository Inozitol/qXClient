#ifndef MESSAGE_H
#define MESSAGE_H

#include "stanza.h"

class Message : public Stanza
{
public:
    Message();
    Message(QXmlStreamReader& reader);
    QString getBody();
    QString getBody(const QLocale& locale);
    QString getSubject();
    QString getSubject(const QLocale& locale);

private:
    QHash<QLocale, QString> _body_map;
    QHash<QLocale, QString> _subj_map;
};

Q_DECLARE_METATYPE(Message);

#endif // MESSAGE_H
