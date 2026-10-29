#pragma once
#include <QDialog>
#include <QString>
class LoginDialog : public QDialog {
public:
    QString userRole;
    int userId=-1;
    QString userName, username;
    LoginDialog();
};
