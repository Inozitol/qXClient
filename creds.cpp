#include "creds.h"

Credentials::Credentials(QByteArray pass)
    : _pass(pass){}

QByteArray Credentials::hashIterate(QCryptographicHash::Algorithm hash, unsigned int iter){
    QByteArray result = _pass;
    for(unsigned int i=0; i<iter; i++){
        result = QCryptographicHash::hash(result, hash);
    }
    return result;
}

const QByteArray &Credentials::getPass() const
{
    return _pass;
}
