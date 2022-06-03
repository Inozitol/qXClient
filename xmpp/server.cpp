#include "server.h"

Server::Server(const QByteArray& domain, quint16 port, const QByteArray& local, const QByteArray& resource)
    : Addressable(domain, local, resource), _port(port)
{}

Server::Server(const jidfull_t& jid, quint16 port)
    : Addressable(jid), _port(port)
{}

quint16 Server::port(){
    return _port;
}
