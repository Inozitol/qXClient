#include "stanza.h"

Stanza::Stanza(Type type)
    : _timestamp(QDateTime::currentDateTime()),
      _type(type)
{
    switch(_type){
    case Stanza::Type::INFOQUERY:
        _stanza = createElement("iq");
        break;
    case Stanza::Type::PRESENCE:
        _stanza = createElement("presence");
        break;
    case Stanza::Type::MESSAGE:
        _stanza = createElement("message");
        break;
    }
    appendChild(_stanza);
}

Stanza::Stanza(const Stanza &stanza)
    : QDomDocument(stanza.cloneNode(true).toDocument()),
      _timestamp(stanza._timestamp),
      _type(stanza.type())
{
    _stanza = firstChild();
}

Stanza::Stanza(Stanza &&stanza)
    : QDomDocument(stanza),
      _timestamp(std::move(stanza._timestamp)),
      _type(stanza.type())
{
    _stanza = firstChild();
}

void Stanza::setTo(QByteArray data){
    _stanza.toElement().setAttribute("to", data);
}

void Stanza::setFrom(QByteArray data){
    _stanza.toElement().setAttribute("from", data);
}

void Stanza::setId(QByteArray data){
    _stanza.toElement().setAttribute("id", data);
}

void Stanza::generateID(){
    QByteArray data = Utils::randomString(ID_LEN);
    _stanza.toElement().setAttribute("id", data);
}

void Stanza::setType(QByteArray data){
    _stanza.toElement().setAttribute("type", data);
}

void Stanza::setLang(QByteArray data){
    _stanza.toElement().setAttribute("lang", data);
    _def_locale = QLocale(data);
}

QByteArray Stanza::getTo() const{
    return _stanza.toElement().attribute("to").toUtf8();
}

QByteArray Stanza::getFrom() const{
    return _stanza.toElement().attribute("from").toUtf8();
}

QByteArray Stanza::getID() const{
    return _stanza.toElement().attribute("id").toUtf8();
}

QByteArray Stanza::getType() const{
    return _stanza.toElement().attribute("type").toUtf8();
}

QByteArray Stanza::getLang() const{
    return _stanza.toElement().attribute("lang").toUtf8();
}

QDateTime Stanza::timestamp() const{
    return _timestamp;
}

void Stanza::insertNode(const QDomNode& node){
    _stanza.appendChild(node);
}

QByteArray Stanza::str() const{
    return toByteArray(0).replace('\n', "");
}

QDomNode Stanza::root() const{
    return _stanza;
}

Stanza::Type Stanza::type() const{
    return _type;
}
