#include "accountdialog.h"
#include "ui_accountdialog.h"
#include "ui_loginwidget.h"

AccountDialog::AccountDialog(Purpose purpose, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AccountDialog)
{
    ui->setupUi(this);
    switch(purpose){
    case Purpose::LOGIN:
        LoginWidget* loginWidget = new LoginWidget(this);

        ui->stackedWidget->addWidget(loginWidget);
        ui->stackedWidget->setCurrentWidget(loginWidget);
        break;
    }
}

const Account& AccountDialog::account(){
    return m_account;
}

const Server& AccountDialog::server(){
    return m_server;
}

void AccountDialog::loginPushed(){
    Stream* stream = StreamPool::instance().newStream(m_account,m_server);
    connect(stream, &Stream::coreEstablished,
            this, &QDialog::accept);
}

LoginWidget::LoginWidget(AccountDialog* parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget),
    _label_font("Arial", 25, QFont::Bold),
    m_wigAccDialog(parent)
{
    ui->setupUi(this);

    connect(ui->loginButton, &QPushButton::clicked,
            this, &LoginWidget::createLoginData);
    connect(ui->loginButton, &QPushButton::clicked,
            m_wigAccDialog, &AccountDialog::loginPushed);
}

void LoginWidget::initUI(){
    ui->accLabel->setFont(_label_font);
}

LoginWidget::~LoginWidget()
{
    delete(ui);
}

void LoginWidget::createLoginData(){
    jidbare_t jid(ui->accountLine->text().toUtf8());
    Credentials creds(ui->passwordLine->text().toUtf8());
    quint16 port = ui->portLine->text().toInt();

    m_wigAccDialog->m_account = Account(jid, creds);
    m_wigAccDialog->m_server = Server(jid, port);
}
