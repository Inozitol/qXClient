#include "contactswidget.h"
#include "ui_contactswidget.h"

ContactsWidget::ContactsWidget(ContactTreeModel* model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactsWidget),
    _model(model)
{
    ui->setupUi(this);
    ui->contactsView->setModel(_model);
}

ContactsWidget::~ContactsWidget()
{
    delete ui;
}
