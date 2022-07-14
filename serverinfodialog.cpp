#include "serverinfodialog.h"
#include "ui_serverinfodialog.h"

ServerInfoDialog::ServerInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerInfoDialog)
{
    ui->setupUi(this);
}

ServerInfoDialog::~ServerInfoDialog()
{
    delete ui;
}
