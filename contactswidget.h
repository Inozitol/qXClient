#ifndef CONTACTSWIDGET_H
#define CONTACTSWIDGET_H

#include <QWidget>

#include "xmpp/contacttreemodel.h"

namespace Ui {
class ContactsWidget;
}

class ContactsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactsWidget(ContactTreeModel* model, QWidget *parent = nullptr);
    ~ContactsWidget();

private:
    Ui::ContactsWidget *ui;
    ContactTreeModel* _model;
};

#endif // CONTACTSWIDGET_H
