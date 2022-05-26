#include "account.h"

Account::Account(const QByteArray& domain, const Credentials& cred, const QByteArray& local, const QByteArray& resource)
    : Addressable(domain, local, resource),
      _cred(cred){}

const Credentials &Account::credentials() const
{
    return _cred;
}
