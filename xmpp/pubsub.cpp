#include "pubsub.h"

PubSub::PubSub(std::shared_ptr<Account> account, std::shared_ptr<Server> server, QObject* parent)
    : QObject(parent), m_sptrAccount(account), m_sptrServer(server){}

void PubSub::processInfoQuery(const InfoQuery &iq){
    jidfull_t from = iq.getFrom();
    QDomElement pubsub_node = iq.root().firstChildElement("pubsub");
    // TODO Implement further event types

    QDomElement items_node = pubsub_node.firstChildElement("items");
    QString node = items_node.attribute("node");

    QDomNodeList items_list = items_node.elementsByTagName("item");
    for(int i = 0; i < items_list.length(); ++i){
        QDomElement item_node = items_list.at(i).toElement();
        processItem(item_node, node, from);
    }
}

void PubSub::processEventMessage(const Message& msg){
    jidfull_t from = msg.getFrom();
    QDomElement event_node = msg.root().firstChildElement("event");
    // TODO Implement further event types

    QDomElement items_node = event_node.firstChildElement("items");
    QString node = items_node.attribute("node");

    QDomNodeList items_list = items_node.elementsByTagName("item");
    for(int i = 0; i < items_list.length(); ++i){
        QDomElement item_node = items_list.at(i).toElement();
        processItem(item_node, node, from);
    }
}

void PubSub::processItem(const QDomElement& item, const QString& node, jidfull_t& from){
    switch(word2int(node.toUtf8())){
        case IntFromString::ns_xmpp_avatar_metadata:
        {
            QDomElement metadata_node = item.firstChildElement("metadata");
            QDomElement info_node = metadata_node.firstChildElement("info");
            QString id = info_node.attribute("id");
            QImage img = DataHolder::instance().getAvatar(id);
            if(img.isNull()){
                requestItem("urn:xmpp:avatar:data", id, from);
            }
        }
        break;

    case IntFromString::ns_xmpp_avatar_data:
        {
            QDomElement data_node = item.firstChildElement("data");
            QString id = item.attribute("id");
            QByteArray png_data = QByteArray::fromBase64(data_node.text().toUtf8());
            QImage avatar;
            if(avatar.loadFromData(png_data)){
                DataHolder::instance().insertAvatar(avatar, id);
                emit avatarUpdated(from, id);
            }
        }
        break;

    case IntFromString::invalid:
        qDebug() << "Node named [" << node << "] found";
        break;
    default:
        qDebug() << "Unimplemented node named [" << node << "] found";
        break;
    }
}

void PubSub::requestItem(const QString& node, const QString& id, jidfull_t& to){
    InfoQuery iq;
    iq.setFrom(m_sptrAccount->jid().toByteArray());
    iq.setTo(to.toByteArray());
    iq.generateID();
    iq.setType("get");

    QDomElement pubsub_node = iq.createElementNS("http://jabber.org/protocol/pubsub", "pubsub");
    QDomElement items_node = iq.createElement("items");
    items_node.setAttribute("node", node);
    QDomElement item_node = iq.createElement("item");
    item_node.setAttribute("id", id);

    iq.root().appendChild(pubsub_node);
    pubsub_node.appendChild(items_node);
    items_node.appendChild(item_node);

    emit sendInfoQuery(iq);
}

void PubSub::subscribe(jidfull_t jid, QByteArray node){
    InfoQuery iq_sub;
    iq_sub.setFrom(m_sptrAccount->jid().toByteArray());
    if(m_mainNode.flgFeatures.testFlag(Disco::Feature::pubsub_subscribe)){
        iq_sub.setTo(m_mainNode.jid.toByteArray());
    }else{
        return;
    }
    iq_sub.generateID();
    iq_sub.setType("set");

    QDomElement pubsub = iq_sub.createElementNS("http://jabber.org/protocol/pubsub","pubsub");
    QDomElement subscribe = iq_sub.createElement("subscribe");
    subscribe.setAttribute("node", node);
    subscribe.setAttribute("jid", jid.toByteArray());
    pubsub.appendChild(subscribe);
    iq_sub.insertNode(pubsub);

    emit sendInfoQuery(iq_sub);
}

void PubSub::pubsubNodeDiscovered(const discoitem_t& item){
    m_pubsubNodes.insert({item.jid, item});
    if(m_mainNode.jid.toByteArray().isEmpty()){
        m_mainNode = item;
    }
    if(item.flgIdentities.testFlag(Disco::Identity::pubsub_service)){
        m_mainNode = item;
    }
}
