#ifndef CONTACTTREEMODEL_H
#define CONTACTTREEMODEL_H

#include <QAbstractListModel>

#include "contact.h"
#include "treeitem.h"

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
    //bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    //bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    //bool setData(const QModelIndex& parent, const QVariant& value, int role = Qt::EditRole);

    void addContact(const Contact& contact);
    QModelIndex indexByJid(const jidbare_t& jid) const;
    QModelIndex indexOf(ContactItem*) const;

    Presence getPresence(const jidfull_t& jid) const;
    rosteritem_t getRoster(const jidbare_t& jid) const;

    void setRoster(const rosteritem_t& roster);
    void insertPresence(const Presence& presence);
private:
    ContactItem* _rootItem;
    ContactItem* _selfItem;
    ContactItem* _o2oSpacer;
    ContactItem* _o2mSpacer;
    jidfull_t _selfJid;
    QMap<jidbare_t, ContactItem*> _jidContacts;
signals:
    void newMessage(const Message& msg);
};

#endif // CONTACTTREEMODEL_H
