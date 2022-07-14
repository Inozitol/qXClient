#include "creds.h"

Credentials::Credentials(QByteArray pass)
    : _pass(pass){}

void Credentials::setPass(QByteArray pass){
    _pass = pass;
}

const QByteArray &Credentials::getPass() const
{
    return _pass;
}
