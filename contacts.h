#ifndef CONTACTS_H
#define CONTACTS_H

#include <QMap>

#include "addressable.h"
#include "account.h"
#include "roster.h"
#include "presence.h"

class Contacts
{
public:
    Contacts(std::shared_ptr<Account> acc);
    void setContact(const jid_t& jid, const rosteritem_t& roster);
    rosteritem_t getContact(const jid_t& jid) const;

    void setPresence(const jid_t& jid, const presence_t& presence);
    presence_t getPresence(const jid_t& jid) const;

private:
    std::shared_ptr<Account> _acc;
    QMap<jid_t, rosteritem_t> _contacts;
    QMap<jid_t, presence_t> _presences;
};

#endif // CONTACTS_H
