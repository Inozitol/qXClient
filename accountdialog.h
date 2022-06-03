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

    explicit AccountDialog(Purpose purpose, QWidget* parent = nullptr);

private:
    Ui::AccountDialog* ui;
private slots:
    void loginSuccessful();
};

class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent = nullptr);
    ~LoginWidget();

private:
    Ui::LoginWidget* ui;

    void initUI();

    QFont _label_font;
private slots:
    void loginPushed();
signals:
    void loginSuccessful();
};

#endif // ACCOUNTDIALOG_H
