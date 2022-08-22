#include "contact.h"

Contact::Contact(QObject* parent)
    : QObject(parent)
{}

Contact::Contact(const rosteritem_t& roster, QObject* parent)
    : QObject(parent),
      m_roster(roster)
      //_chat_chain(new ChatChain(this))
{}

Contact::Contact(const Contact &contact, QObject* parent)
    : QObject(parent)
{
    m_roster = contact.m_roster;
    _presences = contact._presences;
    m_isOnline = contact.m_isOnline;
    m_avatarId = contact.m_avatarId;
    //_chat_chain = contact._chat_chain;
}

Contact::Contact(Contact&& contact, QObject* parent)
    : QObject(parent)
{
    m_roster = contact.m_roster;
    _presences = contact._presences;
    m_isOnline = contact.m_isOnline;
    m_avatarId = contact.m_avatarId;
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

    m_roster = contact.m_roster;
    _presences = contact._presences;
    m_isOnline = contact.m_isOnline;
    m_avatarId = contact.m_avatarId;
    //delete _chat_chain;
    //std::copy(contact._chat_chain, contact._chat_chain, _chat_chain);

    return *this;
}

Contact& Contact::operator=(Contact&& contact) noexcept
{
    if(this == &contact){
        return *this;
    }

    m_roster = contact.m_roster;
    _presences = contact._presences;
    m_isOnline = contact.m_isOnline;

    //delete _chat_chain;
    //_chat_chain = std::exchange(contact._chat_chain, nullptr);

    return *this;
}

void Contact::setRoster(const rosteritem_t &roster){
    m_roster = roster;
}

void Contact::insertPresence(const jidfull_t& jid, const Presence& presence){
    _presences[jid.resource] = presence;
    m_isOnline = true;
}

void Contact::insertPresence(const QByteArray& resource, const Presence& presence){
    _presences[resource] = presence;
    m_isOnline = true;
}

void Contact::removePresence(const jidfull_t &jid){
    if(_presences.contains(jid.resource)){
        _presences.remove(jid.resource);
    }
    if(_presences.count() == 0){
        m_isOnline = false;
    }
}

void Contact::removePresence(const QByteArray &resource){
    if(_presences.contains(resource)){
        _presences.remove(resource);
    }
    if(_presences.count() == 0){
        m_isOnline = false;
    }
}

Presence Contact::getPresence(const jidfull_t& jid) const{
    return _presences.value(jid.resource);
}

Presence Contact::getPresence(const QByteArray& resource) const{
    return _presences.value(resource);
}

rosteritem_t Contact::getRoster() const{
    return m_roster;
}

bool Contact::isOnline(){
    return m_isOnline;
}

void Contact::setAvatarId(const QString &id){
    m_avatarId = id;
}

QString Contact::getAvatarId() const{
    return m_avatarId;
}
