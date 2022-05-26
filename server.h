#ifndef SERVER_H
#define SERVER_H

#include <QString>

#include "addressable.h"


class Server : public Addressable
{
public:
  Server(const QByteArray& domain,
         const QByteArray& local="",
         const QByteArray& resource="");
};

#endif // SERVER_H
