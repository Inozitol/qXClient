#ifndef ACCOUNTWIDGET_H
#define ACCOUNTWIDGET_H

#include <QWidget>

#include "xmpp/stream/streampool.h"
#include "xmpp/contacttreemodel.h"
#include "contactswidget.h"
#include "chatwidget.h"

namespace Ui {
class AccountWidget;
}

class AccountWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountWidget(const jidbare_t& jid, ContactTreeModel* contacts, QWidget *parent = nullptr);
    ~AccountWidget();

private:
    void initContacts();
    void initChat();

    Ui::AccountWidget *ui;
    ContactTreeModel* _contacts;
    jidbare_t _jid;

    QMap<jidbare_t, ChatWidget*> _chats;

private slots:
    void contactClicked(const QModelIndex& index);
    void contactChanged(const QModelIndex& left, const QModelIndex& right);
    void itemExpanded(const QModelIndex& index);
    void itemCollapsed(const QModelIndex& index);
};

#endif // ACCOUNTWIDGET_H
