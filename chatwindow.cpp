#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(ChatChainModel* model, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow),
    _model(model)
{
    ui->setupUi(this);
    ui->chatView->setModel(_model);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}
