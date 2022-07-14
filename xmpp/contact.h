#ifndef CONTACT_H
#define CONTACT_H

#include <QObject>
#include <QMap>

#include "addressable.h"
#include "account.h"
#include "roster.h"
#include "stanza/presence.h"
#include "stanza/message.h"

class Contact : public QObject
{
    Q_OBJECT
public:
    Contact(QObject* parent = nullptr);
    Contact(const rosteritem_t& roster, QObject* parent = nullptr);
    Contact(const Contact& contact, QObject* parent = nullptr);
    Contact(Contact&& contact, QObject* parent = nullptr);
    ~Contact();

    Contact& operator=(const Contact& contact);
    Contact& operator=(Contact&& contact) noexcept;

    void setRoster(const rosteritem_t& roster);
    void insertPresence(const jidfull_t& jid, const Presence& presence);
    void insertPresence(const QByteArray& resource, const Presence& presence);
    void removePresence(const jidfull_t& jid);
    void removePresence(const QByteArray& resource);
    Presence getPresence(const jidfull_t& jid) const;
    Presence getPresence(const QByteArray& resource) const;
    rosteritem_t getRoster() const;
    bool isOnline();
    //ChatChain* getChatChain();

    //void insertMessage(const Message& message);
    //void insertMessage(Message&& message);

private:
    rosteritem_t _roster{};
    QHash<QByteArray, Presence> _presences;
    //ChatChain* _chat_chain = nullptr;
    bool m_isOnline = false;

signals:
    void newMessage(const Message& msg);
};

Q_DECLARE_METATYPE(Contact);
Q_DECLARE_METATYPE(Contact*);

#endif // CONTACT_H
