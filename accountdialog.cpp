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
        connect(loginWidget, &LoginWidget::loginSuccessful,
                this, &AccountDialog::loginSuccessful);
        ui->stackedWidget->addWidget(loginWidget);
        ui->stackedWidget->setCurrentWidget(loginWidget);
        break;
    }
}

void AccountDialog::loginSuccessful(){
    accept();
}

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget),
    _label_font("Arial", 25, QFont::Bold)
{
    ui->setupUi(this);
    connect(ui->loginButton, &QPushButton::clicked,
            this, &LoginWidget::loginPushed);
}

void LoginWidget::initUI(){
    ui->accLabel->setFont(_label_font);
}

LoginWidget::~LoginWidget()
{
    delete(ui);
}

void LoginWidget::loginPushed(){
    jidbare_t jid(ui->accountLine->text().toUtf8());
    Credentials creds(ui->passwordLine->text().toUtf8());
    quint16 port = ui->portLine->text().toInt();

    std::shared_ptr<Account> acc = std::make_shared<Account>(jid, creds);
    std::shared_ptr<Server> srv = std::make_shared<Server>(jid, port);
    Stream* stream = StreamPool::instance().newStream(acc, srv);
    connect(stream, &Stream::coreEstablished,
            this, &LoginWidget::loginSuccessful);
}
