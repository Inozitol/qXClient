#include "accountwidget.h"
#include "ui_accountwidget.h"

AccountWidget::AccountWidget(const jidbare_t& jid, ContactTreeModel* contacts, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::AccountWidget),
      _contacts(contacts),
      _jid(jid)
{
    ui->setupUi(this);
    initContacts();
    initChat();
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);
}

AccountWidget::~AccountWidget()
{
    delete(ui);
}

void AccountWidget::initContacts(){
    ui->contactView->setModel(_contacts);

    connect(ui->contactView, &QAbstractItemView::clicked,
            this, &AccountWidget::contactClicked);
    connect(ui->contactView, &QTreeView::expanded,
            this, &AccountWidget::itemExpanded);
    connect(ui->contactView, &QTreeView::collapsed,
            this, &AccountWidget::itemCollapsed);
    connect(_contacts,  &QAbstractItemModel::dataChanged,
            this, &AccountWidget::contactChanged);
}

void AccountWidget::initChat(){
    ChatWidget* chat = new ChatWidget(Contact(), this);
    chat->setDisabled(true);
    ui->chatStack->addWidget(chat);
    ui->chatStack->setCurrentWidget(chat);
}

void AccountWidget::contactClicked(const QModelIndex& index){
    auto contact_item = static_cast<TreeItem<Contact>*>(index.internalPointer());
    if(contact_item->type() != TreeItem<Contact>::Type::DATA){
        return;
    }
    Contact contact = contact_item->data();

    jidbare_t jid = index.data(Qt::UserRole).toByteArray();

    if(_chats.contains(jid)){
        ui->chatStack->setCurrentWidget(_chats.value(jid));
    }else{
        ChatWidget* chat = new ChatWidget(contact, this);
        Stream* stream = StreamPool::instance().getStream(_jid);

        auto conn = connect(stream->chatChain(jid), &ChatChain::synchronizeMessages,
                chat, &ChatWidget::receiveMessage,
                Qt::QueuedConnection);

        disconnect(conn);

        connect(stream->chatChain(jid), &ChatChain::receivedMessage,
                chat, &ChatWidget::receiveMessage,
                Qt::QueuedConnection);

        connect(chat,&ChatWidget::sendMessage,
                stream->chatChain(jid), &ChatChain::prepareMessage,
                Qt::QueuedConnection);

        ui->chatStack->addWidget(chat);
        ui->chatStack->setCurrentWidget(chat);

        _chats.insert(jid, chat);
    }
}

void AccountWidget::contactChanged(const QModelIndex& left, const QModelIndex& right){
    if(left == right){
        auto contact_item = static_cast<TreeItem<Contact>*>(left.internalPointer());
        Contact contact = contact_item->data();
        jidbare_t jid = left.data(Qt::UserRole).toByteArray();

        if(_chats.contains(jid)){
            _chats.value(jid)->updateMetadata(contact);
        }
    }
}

void AccountWidget::itemExpanded(const QModelIndex &index){
    _contacts->setExpanded(index, true);
}

void AccountWidget::itemCollapsed(const QModelIndex &index){
    _contacts->setExpanded(index, false);
}
