#include "stanza.h"

Stanza::Stanza(const QByteArray& type){
    _stanza = createElement(type);
    appendChild(_stanza);
}

Stanza::Stanza(QByteArray&& type){
    _stanza = createElement(type);
    appendChild(_stanza);
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

void Stanza::setId(){
    QByteArray data = Utils::getRandomString(ID_LEN);
    _stanza.toElement().setAttribute("id", data);
}

void Stanza::setType(QByteArray data){
    _stanza.toElement().setAttribute("type", data);
}

void Stanza::setLang(QByteArray data){
    _stanza.toElement().setAttribute("lang", data);
}

QByteArray Stanza::getTo(){
    return _stanza.toElement().attribute("to").toUtf8();
}

QByteArray Stanza::getFrom(){
    return _stanza.toElement().attribute("from").toUtf8();
}

QByteArray Stanza::getId(){
    return _stanza.toElement().attribute("id").toUtf8();
}

QByteArray Stanza::getType(){
    return _stanza.toElement().attribute("type").toUtf8();
}

QByteArray Stanza::getLang(){
    return _stanza.toElement().attribute("lang").toUtf8();
}

void Stanza::insertNode(const QDomNode& node){
    _stanza.appendChild(node);
}

QByteArray Stanza::str(){
    return toByteArray(0).replace('\n', "");
}

QDomNode Stanza::root(){
    return _stanza;
}
