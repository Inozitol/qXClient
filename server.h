#ifndef SERVER_H
#define SERVER_H

#include <QString>

#include "addressable.h"


class Server : public Addressable
{
public:
  Server(QString domain, QString local="",  QString resource="");
};

#endif // SERVER_H
