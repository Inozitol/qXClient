#ifndef CREDS_H
#define CREDS_H

#include <QString>
#include <QCryptographicHash>

class Credentials{
public:
    Credentials(QByteArray pass);
    Credentials() = default;

    void setPass(QByteArray pass);
    const QByteArray &getPass() const;

private:
    QByteArray _pass;

};

#endif // CREDS_H
