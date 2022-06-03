#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMaximized();

    initMenuBar();
    ui->tabWidget->clear();
}

MainWindow::~MainWindow()
{
    delete(ui);
}

void MainWindow::initMenuBar(){
    connect(ui->actionConnect, &QAction::triggered,
            this, &MainWindow::connectAccount);
}

void MainWindow::initMetaTypes(){
    qRegisterMetaType<Message>();
}

void MainWindow::connectAccount(){
    AccountDialog* diag = new AccountDialog(AccountDialog::Purpose::LOGIN, this);
    if(diag->exec()){
        Stream* stream = StreamPool::instance().getLast();

        jidbare_t jid = stream->accountJid();
        ContactTreeModel* model = stream->getContactsModel();
        TabHolder* widget = new TabHolder(jid,model,this);
        ui->tabWidget->addTab(widget, stream->accountJid().str());
        if(!ui->tabWidget->isEnabled()){
            ui->tabWidget->setEnabled(true);
        }
    }
}
