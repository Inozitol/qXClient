#ifndef PUBSUB_H
#define PUBSUB_H

#include <QObject>

#include "addressable.h"
#include "disco.h"
#include "stanza/infoquery.h"
#include "stanza/message.h"
#include "../strswitch.h"
#include "../dataholder.h"

// Handler for PubSub [XEP-0060]
class PubSub : public QObject
{
    Q_OBJECT
public:
    enum class SubState{
        None,
        Pending,
        Unconfigured,
        Subscribed
    };

    explicit PubSub(std::shared_ptr<Account> account, std::shared_ptr<Server> server, QObject *parent = nullptr);

    void processInfoQuery(const InfoQuery& iq);
    void processEventMessage(const Message& msg);

    void requestItem(const QString& node, const QString& id, jidfull_t& to);

    void subscribe(jidfull_t jid, QByteArray node);

private:
    void processItem(const QDomElement& item, const QString& node, jidfull_t& from);

    std::shared_ptr<Account> m_sptrAccount;
    std::shared_ptr<Server> m_sptrServer;

    std::unordered_map<jidfull_t, discoitem_t> m_pubsubNodes;
    discoitem_t m_mainNode;

public slots:
    void pubsubNodeDiscovered(const discoitem_t& item);
signals:
    void sendInfoQuery(const InfoQuery& iq);
    void avatarUpdated(const jidfull_t& from, const QString& id);
};

#endif // PUBSUB_H
