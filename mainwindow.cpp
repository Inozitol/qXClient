#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/appicon.jpg"));

    initMenuBar();
    initMetaTypes();
    initTheme();

    connect(this, &MainWindow::closing,
            &StreamPool::instance(), &StreamPool::disconnectAll);

    initSettings();
}

MainWindow::~MainWindow()
{
    delete(ui);
}

void MainWindow::initSettings(){
    if(m_settings.value("mainwindow/maximized", 1).toBool()){
        showMaximized();
    }

    if(m_settings.contains("account/map")){
        using namespace Settings::Account;
        QMap<QString, QVariant> qmapAccounts = m_settings.value("account/map").toMap();
        for(auto it = qmapAccounts.cbegin(); it != qmapAccounts.cend(); ++it){
            jidbare_t ujid = it.key();
            QList<QVariant> data = it.value().toList();
            if(data.length() != 3){
                continue;
            }
            jidbare_t sjid = data.at(Index::sjid).toByteArray();
            quint16 port = data.at(Index::port).toUInt();
            QByteArray enCreds = data.at(Index::creds).toByteArray();
            QByteArray deCreds = Utils::simpleEncryption(enCreds, ujid.toByteArray());

            Account account(ujid, Credentials(deCreds));
            Server server(sjid, port);
            Stream* stream = StreamPool::instance().newStream(std::move(account), std::move(server));
            connect(stream, &Stream::coreEstablished,
                    this, [this, stream](){createAccountWidget(stream);});
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event){
    if(isMaximized()){
        m_settings.setValue("mainwindow/maximized", 1);
    }else{
        m_settings.setValue("mainwindow/maximized", 0);
    }

    emit closing();
    QMainWindow::closeEvent(event);
}

void MainWindow::initMenuBar(){
    connect(ui->actionConnect, &QAction::triggered,
            this, &MainWindow::connectAccount);
}

void MainWindow::initMetaTypes(){
    qRegisterMetaType<Message>();
}

void MainWindow::initTheme(){
    QFile f(":qdarkstyle/dark/darkstyle.qss");

    if(!f.exists()){
        qDebug() << "Unable to open stylesheet, file not found";
    }else{
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
}

void MainWindow::connectAccount(){
    AccountDialog* diag = new AccountDialog(AccountDialog::Purpose::LOGIN, this);
    if(diag->exec()){
        Stream* stream = StreamPool::instance().lastStream();

        Account account = diag->account();
        Server server = diag->server();

        // Persistence handling
        QMap<QString, QVariant> qmapAccounts = m_settings.value("account/map", {}).toMap();

        using namespace Settings::Account;

        QList<QVariant> data{};
        data.insert(Index::sjid, server.jid().toByteArray());
        data.insert(Index::port, server.getPort());

        QByteArray deCreds = account.credentials().getPass();
        QByteArray ujid = account.jid().toByteArray();

        QByteArray enCreds = Utils::simpleEncryption(deCreds, ujid);
        data.insert(Index::creds, enCreds);

        qmapAccounts[ujid] = data;

        // Saving login info
        m_settings.setValue("account/map", qmapAccounts);

        createAccountWidget(stream);
    }
}

void MainWindow::createAccountWidget(Stream* stream){
    jidbare_t jid = stream->accountJid();
    ContactTreeModel* model = stream->contactsModel();
    AccountWidget* tab = new AccountWidget(jid,model,this);

    int index = ui->accountStack->addWidget(tab);
    m_tabReference.insert(jid,index);
    ui->accountStack->setCurrentIndex(index);

    if(!ui->accountStack->isEnabled()){
        ui->accountStack->setEnabled(true);
    }
}
