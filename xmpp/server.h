#ifndef SERVER_H
#define SERVER_H

#include <QString>

#include "addressable.h"

class Server : public Addressable
{
public:
    Server(const QByteArray& domain, quint16 port);
    Server(const jidbare_t& jid, quint16 port);
    Server() = default;

    void setPort(quint16 port);
    quint16 getPort();

private:
    quint16 _port;
};

#endif // SERVER_H
