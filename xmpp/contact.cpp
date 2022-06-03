#include "contact.h"

Contact::Contact(QObject* parent)
    : QObject(parent)
{}

Contact::Contact(const rosteritem_t& roster, QObject* parent)
    : QObject(parent),
      _roster(roster)
      //_chat_chain(new ChatChain(this))
{}

Contact::Contact(const Contact &contact, QObject* parent)
    : QObject(parent)
{
    _roster = contact._roster;
    _presences = contact._presences;
    //_chat_chain = contact._chat_chain;
}

Contact::Contact(Contact&& contact, QObject* parent)
    : QObject(parent)
{
    _roster = contact._roster;
    _presences = contact._presences;
    //_chat_chain = contact._chat_chain;
    //contact._chat_chain = nullptr;
}

Contact::~Contact(){
    //delete(_chat_chain);
}

Contact& Contact::operator=(const Contact& contact)
{
    if(this == &contact){
        return *this;
    }

    _roster = contact._roster;
    _presences = contact._presences;

    //delete _chat_chain;
    //std::copy(contact._chat_chain, contact._chat_chain, _chat_chain);

    return *this;
}

Contact& Contact::operator=(Contact&& contact) noexcept
{
    if(this == &contact){
        return *this;
    }

    _roster = contact._roster;
    _presences = contact._presences;

    //delete _chat_chain;
    //_chat_chain = std::exchange(contact._chat_chain, nullptr);

    return *this;
}

void Contact::setRoster(const rosteritem_t &roster){
    _roster = roster;
}

void Contact::insertPresence(const jidfull_t& jid, const Presence& presence){
    _presences[jid.resource] = presence;
}

void Contact::insertPresence(const QByteArray& resource, const Presence& presence){
    _presences[resource] = presence;
}

Presence Contact::getPresence(const jidfull_t& jid) const{
    return _presences.value(jid.resource);
}

Presence Contact::getPresence(const QByteArray& resource) const{
    return _presences.value(resource);
}

rosteritem_t Contact::getRoster() const{
    return _roster;
}

