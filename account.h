#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>

#include "addressable.h"
#include "creds.h"

class Account : public Addressable
{
public:
    Account(const QByteArray& domain,
            const Credentials& cred,
            const QByteArray& local="",
            const QByteArray& resource="");
    const Credentials &credentials() const;

private:
  Credentials _cred;
};

#endif // ACCOUNT_H
