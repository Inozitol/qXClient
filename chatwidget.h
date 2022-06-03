#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

#include "xmpp/chatchain.h"
#include "xmpp/contact.h"

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    ChatWidget(const Contact& contact, QWidget *parent = nullptr);
    ~ChatWidget();

private:
    void initSignals();
    void initLabels(const Contact& contact);

    Ui::ChatWidget *ui;

public slots:
    void receiveMessage(const Message& msg);

private slots:
    void prepareMessage();

signals:
    void sendMessage(Message& msg);
};

#endif // CHATWIDGET_H
