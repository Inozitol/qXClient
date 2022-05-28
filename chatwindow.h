#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include "chatchainmodel.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWindow(ChatChainModel* model, QWidget *parent = nullptr);
    ~ChatWindow();

private:
    Ui::ChatWindow* ui;
    ChatChainModel* _model;
};

#endif // CHATWINDOW_H
