#include "contacts.h"

Contacts::Contacts(std::shared_ptr<Account> acc)
    :_acc(acc)
{}

void Contacts::setContact(const jid_t& jid, const rosteritem_t& roster){
    if(_acc->jid().bare() == jid) return;
    _contacts.insert(jid, roster);
}

rosteritem_t Contacts::getContact(const jid_t& jid) const{
    return _contacts.value(jid);
}

void Contacts::setPresence(const jid_t& jid, const presence_t& presence){
    if(_acc->jid().bare() == jid.bare()) return;
    _presences[jid] = presence;
}

presence_t Contacts::getPresence(const jid_t& jid) const{
    return _presences.value(jid);
}
