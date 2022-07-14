#ifndef CHATCHAIN_H
#define CHATCHAIN_H

#include <QObject>
#include <QAbstractListModel>

#include "stanza/message.h"
#include "addressable.h"

class Stream;

class ChatChain : public QObject
{
    Q_OBJECT
public:
    ChatChain(Stream* stream, jidfull_t receiver, QObject* parent = nullptr);

    ChatChain& operator=(const ChatChain& model);
    ChatChain& operator=(ChatChain&& model) noexcept;

    void addMessage(const Message& msg);
    void addMessage(Message&& msg);

private:
    void includeMessage(const Message& msg);
    void includeMessage(Message&& msg);

    void connectNotify(const QMetaMethod& signal);

    QList<Message> _msgList{};
    Stream* _stream;
    jidfull_t _receiver;
    QByteArray _type = "normal";
public slots:
    void prepareMessage(Message& msg);

signals:
    void receivedMessage(const Message& msg);
    void sendMessage(Message& msg);
    void synchronizeMessages(const Message& msg);
};

#endif // CHATCHAIN_H
