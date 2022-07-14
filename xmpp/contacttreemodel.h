#ifndef CONTACTTREEMODEL_H
#define CONTACTTREEMODEL_H

#include <QAbstractListModel>

#include "contact.h"
#include "treeitem.h"
#include "../dataholder.h"

class ContactTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    using ContactItem = TreeItem<Contact>;

    explicit ContactTreeModel(jidfull_t selfJid, QObject *parent = nullptr);
    ~ContactTreeModel();

    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    void addContact(const Contact& contact);
    QModelIndex indexByJid(const jidbare_t& jid) const;
    QModelIndex indexOf(ContactItem*) const;

    Presence getPresence(const jidfull_t& jid) const;
    rosteritem_t getRoster(const jidbare_t& jid) const;

    void setRoster(const rosteritem_t& roster);
    void insertPresence(const Presence& presence);
    void removePresence(const Presence& presence);

    void setExpanded(const QModelIndex& index, bool value);

private:
    ContactItem* _rootItem;
    ContactItem* _selfItem;
    ContactItem* _o2oSpacer;
    ContactItem* _o2mSpacer;
    jidfull_t _selfJid;
    std::unordered_map<jidbare_t, ContactItem*> m_umapContacts;
    static const QHash<QString, QIcon> m_iconReference;
signals:
    void newMessage(const Message& msg);
};

#endif // CONTACTTREEMODEL_H
