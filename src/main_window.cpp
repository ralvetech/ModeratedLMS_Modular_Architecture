#include "lms/ui/main_window.hpp"
#include "lms/common.hpp"
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

MainWindow::MainWindow(int id, QString r, QString name, QString user):uid(id),role(r),fullName(name),uname(user) {
        setWindowTitle("Moderated LMS | "+roleName(role)); resize(1120,720);
        setStyleSheet("QMainWindow{background:#f3f6fa;} QTabWidget::pane{border:1px solid #d7e0ea;background:white;} QTabBar::tab{padding:10px 16px;} QPushButton{padding:6px 10px;} QTableWidget{background:white;gridline-color:#e2e8f0;} QHeaderView::section{background:#eaf0f6;padding:6px;border:0;font-weight:600;}");
        auto *central=new QWidget; auto *root=new QVBoxLayout(central);
        auto *top=new QHBoxLayout; auto *brand=new QLabel("LMS  /  "+roleName(role).toUpper()); brand->setStyleSheet("font-size:17px;font-weight:800;color:#16324f;");
        auto *who=new QLabel(fullName+"  •  "+uname); who->setStyleSheet("color:#526477;");
        auto *logout=new QPushButton("Sign out"); top->addWidget(brand); top->addStretch(); top->addWidget(who); top->addWidget(logout); root->addLayout(top);
        auto *welcome=new QLabel("Welcome back, "+fullName+". Manage learning and assessment from one place.");
        welcome->setStyleSheet("padding:12px;background:#e5effa;border-radius:5px;color:#1d456b;"); root->addWidget(welcome);
        tabs=new QTabWidget; root->addWidget(tabs); setCentralWidget(central);
        connect(logout,&QPushButton::clicked,this,[this](){close();});
        if(role=="admin") buildAdmin();
        if(role=="lecturer") buildLecturer();
        if(role=="hod") buildHod();
        if(role=="student") buildStudent();
        statusBar()->showMessage("Offline mode • SQLite local database • v1.0");
    }

QWidget *MainWindow::page(const QString &title, const QString &subtitle, QVBoxLayout **out) {
        auto *w=new QWidget; auto *l=new QVBoxLayout(w); auto *h=new QLabel(title); h->setStyleSheet("font-size:19px;font-weight:700;color:#173a5e;padding-top:8px;");
        l->addWidget(h); auto *s=new QLabel(subtitle); s->setStyleSheet("color:#64748b;padding-bottom:5px;"); l->addWidget(s); if(out)*out=l; return w;
    }

QTableWidget *MainWindow::table(QVBoxLayout *l, const QStringList &headers) {
        auto *t=new QTableWidget; t->setColumnCount(headers.size()); t->setHorizontalHeaderLabels(headers); t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); t->setSelectionBehavior(QAbstractItemView::SelectRows); t->setEditTriggers(QAbstractItemView::NoEditTriggers); t->setAlternatingRowColors(true);
        t->setStyleSheet("QTableWidget { color:#111827; background:#ffffff; alternate-background-color:#f3f6fa; gridline-color:#d1d5db; selection-background-color:#bfdbfe; selection-color:#111827; } QTableWidget::item { color:#111827; padding:4px; } QHeaderView::section { color:#111827; background:#e5e7eb; font-weight:600; padding:5px; border:1px solid #d1d5db; }");
        l->addWidget(t,1); return t;
    }

void MainWindow::fill(QTableWidget *t, const QString &sql, int columns) {
        t->setRowCount(0); QSqlQuery q(sql);
        while(q.next()){ int row=t->rowCount(); t->insertRow(row); for(int c=0;c<columns;++c)t->setItem(row,c,new QTableWidgetItem(q.value(c).toString())); }
    }

void MainWindow::refreshSubmissions(QTableWidget *t, bool departmentOnly, bool studentOnly) {
        QString sql="SELECT s.id,c.code,a.title,u.full_name,s.submitted_at,s.status,COALESCE(s.raw_mark,''),COALESCE(s.final_mark,''),s.feedback FROM submissions s JOIN assignments a ON a.id=s.assignment_id JOIN courses c ON c.id=a.course_id JOIN users u ON u.id=s.student_id ";
        if(studentOnly) sql+="WHERE s.student_id="+QString::number(uid)+" ";
        else if(role=="lecturer") sql+="WHERE c.lecturer_id="+QString::number(uid)+" ";
        else if(departmentOnly) sql+="JOIN users me ON me.id="+QString::number(uid)+" WHERE c.department_id=me.department_id ";
        sql+="ORDER BY s.submitted_at DESC";
        t->setColumnCount(9); t->setHorizontalHeaderLabels({"ID","Course","Assignment","Student","Submitted","Status","Raw mark","Final mark","Feedback"}); fill(t,sql,9);
    }
