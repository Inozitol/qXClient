#ifndef STANZA_H
#define STANZA_H

#include <QtXml>
#include <QByteArray>

#include "utils.h"

class Stanza : public QDomElement
{
public:
    Stanza();
    void setTo(QByteArray data);
    void setFrom(QByteArray data);
    void setId(QByteArray data);
    void setId();
    void setType(QByteArray data);
    void setLang(QByteArray data);
    QByteArray getTo();
    QByteArray getFrom();
    QByteArray getId();
    QByteArray getType();
    QByteArray getLang();

    static const unsigned int ID_LEN = 5;
};

#endif // STANZA_H
