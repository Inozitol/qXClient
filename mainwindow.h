#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "xmpp/account.h"
#include "xmpp/server.h"
#include "xmpp/contacttreemodel.h"
#include "xmpp/stream/stream.h"
#include "accountdialog.h"
#include "contactswidget.h"
#include "tabholder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initMenuBar();
    void initMetaTypes();
    Ui::MainWindow *ui;
private slots:
    void connectAccount();
};
#endif // MAINWINDOW_H
