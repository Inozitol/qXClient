#include "contacttreemodel.h"

ContactTreeModel::ContactTreeModel(jidfull_t selfJid, QObject *parent)
    : QAbstractItemModel(parent), _selfJid(selfJid)
{
    _rootItem = new ContactItem(ContactItem::Type::ROOT, Contact());
    _selfItem = new ContactItem(ContactItem::Type::DATA, Contact(), _rootItem);
    _o2oSpacer = new ContactItem(ContactItem::Type::SPACER, Contact(), _rootItem);
    _o2mSpacer = new ContactItem(ContactItem::Type::SPACER, Contact(), _rootItem);
    _rootItem->appendChild(_selfItem);
    _rootItem->appendChild(_o2oSpacer);
    _rootItem->appendChild(_o2mSpacer);
}

ContactTreeModel::~ContactTreeModel(){
    delete(_rootItem);
}

QModelIndex ContactTreeModel::index(int row, int column, const QModelIndex& parent) const{
    if(!hasIndex(row, column, parent)){
        return QModelIndex();
    }

    ContactItem* parentItem;

    if(!parent.isValid()){
        parentItem = _rootItem;
    }else{
        parentItem = static_cast<ContactItem*>(parent.internalPointer());
    }

    ContactItem* childItem = parentItem->child(row);
    if(childItem){
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex ContactTreeModel::parent(const QModelIndex& index) const{
    if(!index.isValid()){
        return QModelIndex();
    }

    ContactItem* childItem = static_cast<ContactItem*>(index.internalPointer());
    ContactItem* parentItem = childItem->parent();

    if(parentItem == _rootItem){
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int ContactTreeModel::rowCount(const QModelIndex& parent) const{
    ContactItem* parentItem;
    if(parent.column() > 0){
        return 0;
    }

    if(!parent.isValid()){
        parentItem = _rootItem;
    }else{
        parentItem = static_cast<ContactItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

QVariant ContactTreeModel::data(const QModelIndex& index, int role) const{
    if(!index.isValid()){
        return QVariant();
    }
    ContactItem* item = static_cast<ContactItem*>(index.internalPointer());
    Contact cnt = item->data();
    switch(role){
    case Qt::DisplayRole:
    {
        if(item == _o2oSpacer){
            return tr("Contacts");
        }
        if(item == _o2mSpacer){
            return tr("Groups");
        }
        jidbare_t jid = cnt.getRoster().jid;
        return jid.local;
    }
        break;
    case Qt::DecorationRole:
        if(item->type() == ContactItem::Type::SPACER){
            if(item->isExpanded()){
                return DataHolder::instance().getIcon("collapse");
            }else{
                return DataHolder::instance().getIcon("expand");
            }
        }
        if(cnt.isOnline()){
            return DataHolder::instance().getIcon("online");
        }else{
            return DataHolder::instance().getIcon("offline");
        }
        break;
    case Qt::UserRole:
    {
        jidbare_t jid = cnt.getRoster().jid;
        return jid.toByteArray();
    }
    case Qt::UserRole+1:
    {
        Contact* cnt = static_cast<ContactItem*>(index.internalPointer())->dataPtr();
        QVariant data;
        data.setValue(cnt);
        return data;
    }
        break;
    }

    return QVariant();
}

Qt::ItemFlags ContactTreeModel::flags(const QModelIndex& index) const{
    if(!index.isValid()){
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}

int ContactTreeModel::columnCount(const QModelIndex&) const{
    return 1;
}

void ContactTreeModel::addContact(const Contact& contact){
    const jidbare_t jid = contact.getRoster().jid;

    if(jid == _selfJid.bare()){
        _selfItem->setData(contact);
        m_umapContacts.insert({jid, _selfItem});
        QModelIndex selfIndex = index(0, 0, QModelIndex());
        emit dataChanged(selfIndex, selfIndex);
    }else{
        ContactItem* item = new ContactItem(ContactItem::Type::DATA, contact, _o2oSpacer);
        _o2oSpacer->appendChild(item);
        m_umapContacts.insert({jid, item});
    }
}

QModelIndex ContactTreeModel::indexByJid(const jidbare_t& jid) const{
    ContactItem* item;
    try{
        item = m_umapContacts.at(jid);
    }catch(const std::out_of_range&){
        item = nullptr;
    }
    QModelIndex index = indexOf(item);
    return index;
}

QModelIndex ContactTreeModel::indexOf(ContactItem* item) const{
    if(!item){
        return QModelIndex();
    }
    // TODO Do this non recusively
    if(item == _rootItem){
        return QModelIndex();
    }
    QModelIndex parentIndex = indexOf(item->parent());
    return index(item->row(), 0, parentIndex);
}

void ContactTreeModel::insertPresence(const Presence& presence){
    jidbare_t jid = presence.getFrom();
    QModelIndex index = indexByJid(jid);
    if(!index.isValid()){
        rosteritem_t roster;
        roster.jid = jid;
        Contact contact(roster);
        addContact(contact);
        index = indexByJid(jid);
    }
    ContactItem* item = static_cast<ContactItem*>(index.internalPointer());
    item->dataPtr()->insertPresence(jidfull_t(presence.getFrom()), presence);
    emit dataChanged(index, index);
}

void ContactTreeModel::removePresence(const Presence &presence){
    jidbare_t jid = presence.getFrom();
    QModelIndex index = indexByJid(jid);
    if(!index.isValid()){
        rosteritem_t roster;
        roster.jid = jid;
        Contact contact(roster);
        addContact(contact);
        index = indexByJid(jid);
    }
    ContactItem* item = static_cast<ContactItem*>(index.internalPointer());
    item->dataPtr()->removePresence(jidfull_t(presence.getFrom()));
    emit dataChanged(index, index);
}

void ContactTreeModel::updateAvatarId(const jidfull_t &jid, const QString &id){
    QModelIndex index = indexByJid(jid);
    if(!index.isValid()){
        return;
    }
    ContactItem* item = static_cast<ContactItem*>(index.internalPointer());
    item->dataPtr()->setAvatarId(id);
    emit dataChanged(index, index);
}

void ContactTreeModel::setExpanded(const QModelIndex &index, bool value){
    ContactItem* item = static_cast<ContactItem*>(index.internalPointer());
    item->setExpanded(value);
    emit dataChanged(index, index);
}

Presence ContactTreeModel::getPresence(const jidfull_t& jid) const{
    QModelIndex index = indexByJid(jid);
    const ContactItem* item = static_cast<const ContactItem*>(index.constInternalPointer());
    return item->data().getPresence(jid);
}

rosteritem_t ContactTreeModel::getRoster(const jidbare_t& jid) const{
    QModelIndex index = indexByJid(jid);
    const ContactItem* item = static_cast<const ContactItem*>(index.constInternalPointer());
    return item->data().getRoster();
}

void ContactTreeModel::setRoster(const rosteritem_t& roster){
    jidbare_t jid = roster.jid;
    QModelIndex index = indexByJid(jid);
    if(!index.isValid()){
        Contact contact(roster);
        addContact(contact);
    }else{
        ContactItem* item = static_cast<ContactItem*>(index.internalPointer());
        item->dataPtr()->setRoster(roster);
        emit dataChanged(index, index);
    }
}
