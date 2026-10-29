#include "lms/ui/login_dialog.hpp"
#include "lms/common.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>

LoginDialog::LoginDialog() {
        setWindowTitle("Moderated LMS — Sign in"); setMinimumWidth(390);
        auto *layout=new QVBoxLayout(this);
        auto *heading=new QLabel("LEARNING MANAGEMENT SYSTEM");
        heading->setStyleSheet("font-size:18px;font-weight:700;color:#16324f;padding:8px 0;");
        layout->addWidget(heading);
        layout->addWidget(new QLabel("Sign in with your school account."));
        auto *form=new QFormLayout;
        auto *u=new QLineEdit; u->setObjectName("username"); u->setPlaceholderText("Username");
        auto *p=new QLineEdit; p->setObjectName("password"); p->setEchoMode(QLineEdit::Password); p->setPlaceholderText("Password");
        form->addRow("Username",u); form->addRow("Password",p); layout->addLayout(form);
        auto *login=new QPushButton("Sign in"); login->setDefault(true);
        login->setStyleSheet("background:#1f5b99;color:white;padding:9px;font-weight:600;border-radius:5px;");
        layout->addWidget(login);
        auto *demo=new QLabel("Demo accounts: admin / lecturer / hod / student\nPassword: ChangeMe123!");
        demo->setStyleSheet("color:#68778a;font-size:11px;padding-top:6px;"); layout->addWidget(demo);
        connect(login,&QPushButton::clicked,this,[=](){
            QSqlQuery q; q.prepare("SELECT id,full_name,role,salt,password_hash,active FROM users WHERE username=?"); q.addBindValue(u->text().trimmed());
            if(!q.exec()||!q.next()||q.value(5).toInt()!=1||!verifyPassword(p->text(),q.value(3).toString(),q.value(4).toString())) {
                QMessageBox::warning(this,"Sign in failed","The username or password is incorrect, or the account is inactive."); return;
            }
            userId=q.value(0).toInt(); userName=q.value(1).toString(); userRole=q.value(2).toString(); username=u->text().trimmed(); accept();
        });
        connect(p,&QLineEdit::returnPressed,login,&QPushButton::click);
    }
