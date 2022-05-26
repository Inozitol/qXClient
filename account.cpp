#include "account.h"

Account::Account(QString domain, const Credentials& cred, QString local, QString resource)
    : Addressable(domain, local, resource),
      _cred(cred){}

const Credentials &Account::credentials() const
{
    return _cred;
}
