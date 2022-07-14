#ifndef SERVERINFODIALOG_H
#define SERVERINFODIALOG_H

#include <QDialog>

namespace Ui {
class ServerInfoDialog;
}

class ServerInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerInfoDialog(QWidget *parent = nullptr);
    ~ServerInfoDialog();

private:
    Ui::ServerInfoDialog *ui;
};

#endif // SERVERINFODIALOG_H
