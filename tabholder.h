#ifndef TABHOLDER_H
#define TABHOLDER_H

#include <QWidget>

#include "xmpp/stream/streampool.h"
#include "xmpp/contacttreemodel.h"
#include "contactswidget.h"
#include "chatwidget.h"

namespace Ui {
class TabHolder;
}

class TabHolder : public QWidget
{
    Q_OBJECT

public:
    explicit TabHolder(const jidbare_t& jid, ContactTreeModel* contacts, QWidget *parent = nullptr);
    ~TabHolder();

private:
    void initContacts();
    void initChat();

    Ui::TabHolder *ui;
    ContactTreeModel* _contacts;
    jidbare_t _jid;

    QMap<jidbare_t, ChatWidget*> _chats;
private slots:
    void contactClicked(const QModelIndex& index);
};

#endif // TABHOLDER_H
