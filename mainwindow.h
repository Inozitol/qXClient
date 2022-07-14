#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "xmpp/stream/stream.h"
#include "xmpp/account.h"
#include "xmpp/server.h"
#include "xmpp/creds.h"
#include "xmpp/contacttreemodel.h"
#include "accountdialog.h"
#include "contactswidget.h"
#include "accountwidget.h"
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent* event);

private:
    void initMenuBar();
    void initMetaTypes();
    void initSettings();
    void initTheme();

    void createAccountWidget(Stream* stream);

    Ui::MainWindow *ui;

    QSettings m_settings;
    QHash<jidbare_t, int> m_tabReference;

private slots:
    void connectAccount();

signals:
    void closing();
};
#endif // MAINWINDOW_H
