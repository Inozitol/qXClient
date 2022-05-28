#ifndef CHATCHAINMODEL_H
#define CHATCHAINMODEL_H

#include <QAbstractListModel>

#include "message.h"
#include "addressable.h"

class ChatChainModel : public QAbstractTableModel
{
public:
    explicit ChatChainModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool setData(const QModelIndex& parent, const QVariant& value, int role = Qt::EditRole);

    void addMessage(const Message& msg);
private:
    QList<Message> _msgList;
};

#endif // CHATCHAINMODEL_H
