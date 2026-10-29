#include "lms/ui/main_window.hpp"
#include "lms/common.hpp"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QDateEdit>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDate>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QDir>
#include <algorithm>

void MainWindow::buildStudent() {
        QVBoxLayout *l;auto *w=page("My learning","Your enrolled courses, current assignments, submissions and released feedback.",&l);
        auto *bar=new QHBoxLayout;auto *submit=new QPushButton("Submit / resubmit work");bar->addWidget(submit);bar->addStretch();l->addLayout(bar);
        auto *a=table(l,{"ID","Course","Assignment","Deadline","Max mark","Description","Submission status","Released mark","Feedback"});
        auto refresh=[=](){
            a->setRowCount(0);QSqlQuery q;q.prepare("SELECT a.id,c.code,a.title,COALESCE(i.due_at,a.class_extension,a.due_at),a.max_mark,a.description,COALESCE(s.status,'Not submitted'),CASE WHEN s.status='Released' THEN CAST(s.final_mark AS TEXT) ELSE 'Not released' END,CASE WHEN s.status='Released' THEN s.feedback ELSE '' END FROM enrolments e JOIN courses c ON c.id=e.course_id JOIN assignments a ON a.course_id=c.id LEFT JOIN submissions s ON s.assignment_id=a.id AND s.student_id=e.student_id LEFT JOIN individual_extensions i ON i.assignment_id=a.id AND i.student_id=e.student_id WHERE e.student_id=? ORDER BY a.due_at");q.addBindValue(uid);q.exec();while(q.next()){int row=a->rowCount();a->insertRow(row);for(int c=0;c<9;++c)a->setItem(row,c,new QTableWidgetItem(q.value(c).toString()));}
        };
        connect(submit,&QPushButton::clicked,this,[=](){
            int row=a->currentRow();if(row<0){QMessageBox::information(this,"Choose assignment","Select an assignment first.");return;}int aid=a->item(row,0)->text().toInt();QString status=a->item(row,6)->text();if(status=="Released"){QMessageBox::warning(this,"Submission locked","This submission has already been released and cannot be changed.");return;}
            QSqlQuery existing;existing.prepare("SELECT id FROM submissions WHERE assignment_id=? AND student_id=?");existing.addBindValue(aid);existing.addBindValue(uid);existing.exec();bool has=existing.next();
            QDialog d(this);d.setWindowTitle(has?"Resubmit work":"Submit assignment");QVBoxLayout v(&d);v.addWidget(new QLabel("Enter your submission text or a description of the locally stored work. This prototype stores text in SQLite; file uploads are not included."));QTextEdit content;content.setMinimumSize(520,220);v.addWidget(&content);QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);v.addWidget(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);if(d.exec()!=QDialog::Accepted||content.toPlainText().trimmed().isEmpty())return;
            QSqlQuery due;due.prepare("SELECT COALESCE(i.due_at,a.class_extension,a.due_at) FROM assignments a LEFT JOIN individual_extensions i ON i.assignment_id=a.id AND i.student_id=? WHERE a.id=?");due.addBindValue(uid);due.addBindValue(aid);due.exec();due.next();QDateTime deadline=QDateTime::fromString(due.value(0).toString(),Qt::ISODate);QString statusNow=QDateTime::currentDateTime()>deadline?"Late":"Submitted";QString now=QDateTime::currentDateTime().toString(Qt::ISODate);
            QSqlQuery x;if(has){x.prepare("UPDATE submissions SET content=?,submitted_at=?,status=?,raw_mark=NULL,final_mark=NULL,feedback='',marked_by=NULL,moderated_by=NULL,released_at=NULL WHERE assignment_id=? AND student_id=?");x.addBindValue(content.toPlainText());x.addBindValue(now);x.addBindValue(statusNow);x.addBindValue(aid);x.addBindValue(uid);}else{x.prepare("INSERT INTO submissions(content,submitted_at,status,assignment_id,student_id) VALUES(?,?,?,?,?)");x.addBindValue(content.toPlainText());x.addBindValue(now);x.addBindValue(statusNow);x.addBindValue(aid);x.addBindValue(uid);}
            if(!x.exec())QMessageBox::warning(this,"Submission failed",x.lastError().text());else QMessageBox::information(this,"Submitted",statusNow=="Late"?"Your work was accepted as late. The late-penalty rule may apply.":"Your work has been submitted.");refresh();
        });
        tabs->addTab(w,"My learning");refresh();
    }
