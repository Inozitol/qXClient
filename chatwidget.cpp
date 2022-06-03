#include "chatwidget.h"
#include "ui_chatwidget.h"

ChatWidget::ChatWidget(const Contact& contact, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWidget)
{
    ui->setupUi(this);
    initSignals();
    initLabels(contact);
}

ChatWidget::~ChatWidget()
{
    delete(ui);
}

void ChatWidget::initSignals(){
    connect(ui->chatInput, &QLineEdit::returnPressed,
            this, &ChatWidget::prepareMessage);
    connect(ui->chatInput, &QLineEdit::returnPressed,
            ui->chatInput, &QLineEdit::clear);
}

void ChatWidget::initLabels(const Contact& contact){
    ui->nameLabel->setText(contact.getRoster().name);
}

void ChatWidget::receiveMessage(const Message& msg){
    jidbare_t fromJid = msg.getFrom();
    QString str = fromJid.local + " : " + msg.getBody();
    ui->chatView->addItem(str);
}

void ChatWidget::prepareMessage(){
    QString text = ui->chatInput->text();
    Message msg;
    msg.setBody(text);
    emit sendMessage(msg);
}
