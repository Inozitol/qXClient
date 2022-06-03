#ifndef STANZA_H
#define STANZA_H

#include <QtXml>
#include <QByteArray>

#include "../../utils.h"
#include "../../strswitch.h"

class Stanza : public QDomDocument
{
public:
    enum class Type{
        INFOQUERY,
        PRESENCE,
        MESSAGE
    };

    Stanza(Type type);
    Stanza(const Stanza& stanza);
    Stanza(Stanza&& stanza);

    Stanza& operator=(const Stanza&)=default;

    void setTo(QByteArray data);
    void setFrom(QByteArray data);
    void setId(QByteArray data);
    void setId();
    void setType(QByteArray data);
    void setLang(QByteArray data);
    QByteArray getTo() const;
    QByteArray getFrom() const;
    QByteArray getId() const;
    QByteArray getType() const;
    QByteArray getLang() const;

    void insertNode(const QDomNode& node);
    QDomNode root();
    QByteArray str();

    static const unsigned int ID_LEN = 5;
protected:
    QDomNode _stanza;
    QLocale _def_locale = QLocale("en");
private:
    QDateTime _timestamp;
    Type _type;

};

#endif // STANZA_H
