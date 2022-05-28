#include "chatchainmodel.h"

ChatChainModel::ChatChainModel(QObject *parent)
    : QAbstractTableModel{parent}
{}

int ChatChainModel::rowCount(const QModelIndex&) const{
    return _msgList.size();
}

int ChatChainModel::columnCount(const QModelIndex&) const{
    return 2;
}

QVariant ChatChainModel::data(const QModelIndex& index, int role) const{
    if(!index.isValid()){
        return QVariant();
    }
    if(role == Qt::DisplayRole){
        switch(index.column()){
            case 0:
                return jid_t(_msgList.value(index.row()).getFrom()).local;
            break;
        case 1:
                return _msgList.value(index.row()).getBody();
            break;
        }
    }
    return QVariant();
}

bool ChatChainModel::insertRows(int row, int count, const QModelIndex&){
    beginInsertRows(QModelIndex(), row, row+count-1);

    while(count--){
        _msgList.insert(row, Message());
    }

    endInsertRows();
    return true;
}

bool ChatChainModel::removeRows(int row, int count, const QModelIndex&){
    beginRemoveRows(QModelIndex(), row, row+count-1);

    while(count--){
        _msgList.removeAt(row);
    }

    endRemoveRows();
    return true;
}

bool ChatChainModel::setData(const QModelIndex& parent, const QVariant& value, int role){
    if(parent.isValid() && role == Qt::EditRole){
        _msgList.replace(parent.row(), qvariant_cast<Message>(value));
        emit dataChanged(parent, parent);
        return true;
    }
    return false;
}

void ChatChainModel::addMessage(const Message &msg){
    insertRow(_msgList.count());
    QModelIndex index = createIndex(_msgList.count()-1, 0);
    QVariant data;
    data.setValue(msg);
    setData(index, data);
}
