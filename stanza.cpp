#include "stanza.h"

Stanza::Stanza(){}

void Stanza::setTo(QByteArray data){
    setAttribute("to", data);
}

void Stanza::setFrom(QByteArray data){
    setAttribute("from", data);
}

void Stanza::setId(QByteArray data){
    setAttribute("id", data);
}

void Stanza::setId(){
    QByteArray data = Utils::getRandomString(ID_LEN);
    setAttribute("id", data);
}

void Stanza::setType(QByteArray data){
    setAttribute("type", data);
}

void Stanza::setLang(QByteArray data){
    setAttribute("lang", data);
}

QByteArray Stanza::getTo(){
    return attribute("to").toUtf8();
}

QByteArray Stanza::getFrom(){
    return attribute("from").toUtf8();
}

QByteArray Stanza::getId(){
    return attribute("id").toUtf8();
}

QByteArray Stanza::getType(){
    return attribute("type").toUtf8();
}

QByteArray Stanza::getLang(){
    return attribute("lang").toUtf8();
}
