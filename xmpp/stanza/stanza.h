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

    Stanza& operator=(const Stanza&) = default;

    void setTo(QByteArray data);
    void setFrom(QByteArray data);
    void setId(QByteArray data);
    void generateID();
    void setType(QByteArray data);
    void setLang(QByteArray data);
    QByteArray getTo() const;
    QByteArray getFrom() const;
    QByteArray getID() const;
    QByteArray getType() const;
    QByteArray getLang() const;

    QDateTime timestamp() const;

    void setAttribute(const QByteArray& name, const QByteArray& content);
    void insertNode(const QDomNode& node);
    QDomNode root() const;
    QByteArray str() const;
    Type type() const;

    static const unsigned int ID_LEN = 5;
protected:
    QDomNode _stanza;
    QLocale _def_locale = QLocale("en");
    QDateTime _timestamp;
private:
    Type _type;
};

#endif // STANZA_H
