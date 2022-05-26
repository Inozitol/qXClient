#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>

#include "addressable.h"
#include "creds.h"

class Account : public Addressable
{
public:
    Account(QString domain, const Credentials& cred, QString local="", QString resource="");
    const Credentials &credentials() const;

private:
  Credentials _cred;
};

#endif // ACCOUNT_H
