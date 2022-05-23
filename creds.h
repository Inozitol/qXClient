#ifndef CREDS_H
#define CREDS_H

#include <QString>
#include <QCryptographicHash>

class Credentials{
public:
    Credentials(QByteArray pass);

    QByteArray hashIterate(QCryptographicHash::Algorithm hash, unsigned int iter);

    const QByteArray &getPass() const;

private:
    QByteArray _pass;

};

#endif // CREDS_H
