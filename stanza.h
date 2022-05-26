#ifndef STANZA_H
#define STANZA_H

#include <QtXml>
#include <QByteArray>

#include "utils.h"
#include "strswitch.h"

class Stanza : public QDomDocument
{
public:
    Stanza(const QByteArray& type);

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

    void insertNode(const QDomNode& node);
    QByteArray str();

    static const unsigned int ID_LEN = 5;
protected:
    QDomNode _stanza;
};

#endif // STANZA_H
