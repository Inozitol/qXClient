#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    std::shared_ptr<Account> cle = std::make_shared<Account>("xmpp.chochrun.eu", Credentials("xmpppass"), "tester");
    std::shared_ptr<Server> srv = std::make_shared<Server>("xmpp.chochrun.eu");

    QThread* thread = new QThread();
    Stream* stream = new Stream(cle, srv);
    stream->moveToThread(thread);
    connect(thread, &QThread::started,
            stream, &Stream::connectInsecure);
    connect(stream, &Stream::finished,
            thread, &QThread::quit);
    connect(stream, &Stream::finished,
            stream, &Stream::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    thread->start();

    ChatWindow* chatW = new ChatWindow(stream->chatModel);
    chatW->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

