#include "chatchain.h"
#include "stream/stream.h"

ChatChain::ChatChain(Stream* stream, jidfull_t receiver, QObject* parent)
    : QObject{parent},
      _msgList(),
      _stream(stream),
      _receiver(receiver)
{}

ChatChain &ChatChain::operator=(const ChatChain& model){
    if(this == &model){
        return *this;
    }

    _msgList = model._msgList;
    return *this;
}

ChatChain &ChatChain::operator=(ChatChain&& model) noexcept{
    if(this == &model){
        return *this;
    }

    _msgList = model._msgList;
    model._msgList.clear();
    return *this;
}

void ChatChain::addMessage(const Message& msg){
    _receiver = msg.getFrom();
    _msgList.append(msg);
    if(!_msgList.constLast().getBody().isEmpty()){
        emit receivedMessage(_msgList.constLast());
    }
}

void ChatChain::addMessage(Message&& msg){
    _receiver = msg.getFrom();
    _msgList.append(std::move(msg));
    if(!_msgList.constLast().getBody().isEmpty()){
        emit receivedMessage(_msgList.constLast());
    }
}

void ChatChain::includeMessage(const Message &msg){
    _msgList.append(msg);
    if(!_msgList.constLast().getBody().isEmpty()){
        emit receivedMessage(_msgList.constLast());
    }
}

void ChatChain::includeMessage(Message &&msg){
    _msgList.append(std::move(msg));
    if(!_msgList.constLast().getBody().isEmpty()){
        emit receivedMessage(_msgList.constLast());
    }
}

void ChatChain::prepareMessage(Message& msg){
    msg.setFrom(_stream->accountJid().str());
    msg.setTo(_receiver.str());
    msg.setType("chat");
    emit sendMessage(msg);
    includeMessage(std::move(msg));
}
