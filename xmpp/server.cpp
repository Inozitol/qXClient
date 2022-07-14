#include "server.h"

Server::Server(const QByteArray& domain, quint16 port)
    : Addressable(domain, "", ""), _port(port)
{}

Server::Server(const jidbare_t& jid, quint16 port)
    : Addressable(jid.domain), _port(port)
{}

void Server::setPort(quint16 port){
    _port = port;
}

quint16 Server::getPort(){
    return _port;
}
