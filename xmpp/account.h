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

    Account(const jidfull_t& jid, const Credentials& cred);
    Account(const jidbare_t& jid, const Credentials& cred);

    const Credentials &credentials() const;

private:
    Credentials _cred;
};

#endif // ACCOUNT_H
