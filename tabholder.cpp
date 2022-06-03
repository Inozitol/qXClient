#include "tabholder.h"
#include "ui_tabholder.h"

TabHolder::TabHolder(const jidbare_t& jid, ContactTreeModel* contacts, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::TabHolder),
      _contacts(contacts),
      _jid(jid)
{
    ui->setupUi(this);
    initContacts();
    initChat();
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);
}

TabHolder::~TabHolder()
{
    delete(ui);
}

void TabHolder::initContacts(){
    ui->contactView->setModel(_contacts);
    connect(ui->contactView, &QAbstractItemView::clicked,
            this, &TabHolder::contactClicked);
}

void TabHolder::initChat(){
    ChatWidget* chat = new ChatWidget(this);
    chat->setDisabled(true);
    ui->chatStack->addWidget(chat);
    ui->chatStack->setCurrentWidget(chat);
}

void TabHolder::contactClicked(const QModelIndex& index){
    jidbare_t jid = index.data(Qt::UserRole).toByteArray();

    if(_chats.contains(jid)){
        ui->chatStack->setCurrentWidget(_chats.value(jid));
    }else{
        ChatWidget* chat = new ChatWidget(this);
        Stream* stream = StreamPool::instance().getStream(_jid);

        connect(stream->getChatChain(jid), &ChatChain::receivedMessage,
                chat, &ChatWidget::receiveMessage,
                Qt::QueuedConnection);

        connect(chat,&ChatWidget::sendMessage,
                stream->getChatChain(jid), &ChatChain::prepareMessage,
                Qt::QueuedConnection);

        ui->chatStack->addWidget(chat);
        ui->chatStack->setCurrentWidget(chat);

        _chats.insert(jid, chat);
    }
}
