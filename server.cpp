#include "server.h"

Server::Server(const QByteArray& domain, const QByteArray& local, const QByteArray& resource)
  : Addressable(domain, local, resource)
{

}
