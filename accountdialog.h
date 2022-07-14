#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QFont>
#include <QThreadPool>
#include <QMessageBox>

#include "xmpp/stream/stream.h"
#include "xmpp/stream/streampool.h"
#include "xmpp/account.h"
#include "xmpp/server.h"

namespace Ui {
class AccountDialog;
class LoginWidget;
}

class AccountDialog : public QDialog{
    Q_OBJECT
public:
    enum class Purpose{
        LOGIN
    };

    AccountDialog(Purpose purpose, QWidget* parent = nullptr);

    const Account& account();
    const Server&  server();

private:
    Ui::AccountDialog* ui;

    Account m_account;
    Server  m_server;

    friend class LoginWidget;

private slots:
    void loginPushed();
};

class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    LoginWidget(AccountDialog* parent = nullptr);
    ~LoginWidget();

private:
    Ui::LoginWidget* ui;

    void initUI();

    QFont _label_font;
    QSettings m_settings;
    AccountDialog* m_wigAccDialog;
private slots:
    void createLoginData();
signals:
    void loginSuccessful();
};

#endif // ACCOUNTDIALOG_H
