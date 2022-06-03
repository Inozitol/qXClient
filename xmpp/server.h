#ifndef SERVER_H
#define SERVER_H

#include <QString>

#include "addressable.h"

class Server : public Addressable
{
public:
    Server(const QByteArray& domain,
           quint16 port,
           const QByteArray& local="",
           const QByteArray& resource="");

    Server(const jidfull_t& jid,
           quint16 port);

    quint16 port();

private:
    quint16 _port;
};

#endif // SERVER_H
