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

ChatWidget::~ChatWidget(){
    delete(ui);
}

void ChatWidget::updateMetadata(const Contact& contact){
    rosteritem_t roster = contact.getRoster();
    if(!roster.name.isEmpty()){
        ui->nameLabel->setText(roster.name);
    }else{
        ui->nameLabel->setText(roster.jid.local);
    }
    if(!contact.getAvatarId().isEmpty()){
        QString id = contact.getAvatarId();
        QImage avatar = DataHolder::instance().getAvatar(id);
        ui->avatarLabel->setPixmap(QPixmap::fromImage(avatar));
        ui->avatarLabel->setScaledContents(true);
    }
}

void ChatWidget::initSignals(){
    connect(ui->chatInput, &QLineEdit::returnPressed,
            this, &ChatWidget::prepareMessage);
    connect(ui->chatInput, &QLineEdit::returnPressed,
            ui->chatInput, &QLineEdit::clear);
}

void ChatWidget::initLabels(const Contact& contact){
    updateMetadata(contact);
}

void ChatWidget::receiveMessage(const Message& msg){
    jidbare_t fromJid = msg.getFrom();
    QString time = msg.timestamp().toString("hh:mm:ss");
    QString str = time + " | " + fromJid.local + " : " + msg.getBody();
    ui->chatView->addItem(str);
}

void ChatWidget::prepareMessage(){
    QString text = ui->chatInput->text();
    Message msg;
    msg.setBody(text);
    emit sendMessage(msg);
}
