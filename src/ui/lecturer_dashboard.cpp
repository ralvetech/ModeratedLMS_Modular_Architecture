#include "lms/ui/main_window.hpp"
#include "lms/domain.hpp"
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

void MainWindow::buildLecturer() {
        QVBoxLayout *l;auto *w=page("Teaching workspace","Create assessment, extend deadlines, mark work and provide feedback.",&l);
        auto *bar=new QHBoxLayout;auto *create=new QPushButton("Create assignment");auto *individual=new QPushButton("Individual extension");auto *whole=new QPushButton("Extend whole class");auto *mark=new QPushButton("Mark selected submission");bar->addWidget(create);bar->addWidget(individual);bar->addWidget(whole);bar->addWidget(mark);l->addLayout(bar);
        auto *t=table(l,{"ID","Course","Assignment","Student","Submitted","Status","Raw mark","Final mark","Feedback"});
        auto refresh=[=](){refreshSubmissions(t);};
        connect(create,&QPushButton::clicked,this,[=](){
            QDialog d(this);d.setWindowTitle("Create assignment");QFormLayout f(&d);QComboBox c;QSqlQuery q;q.prepare("SELECT id,code||' — '||title FROM courses WHERE lecturer_id=?");q.addBindValue(uid);q.exec();while(q.next())c.addItem(q.value(1).toString(),q.value(0));QLineEdit title;QTextEdit desc;QDateTimeEdit due(QDateTime::currentDateTime().addDays(7));due.setCalendarPopup(true);QDoubleSpinBox max;max.setRange(1,10000);max.setValue(100);f.addRow("Course",&c);f.addRow("Title",&title);f.addRow("Description",&desc);f.addRow("Deadline",&due);f.addRow("Maximum mark",&max);QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);if(d.exec()==QDialog::Accepted){QSqlQuery x;x.prepare("INSERT INTO assignments(course_id,title,description,due_at,max_mark,created_by) VALUES(?,?,?,?,?,?)");x.addBindValue(c.currentData());x.addBindValue(title.text().trimmed());x.addBindValue(desc.toPlainText());x.addBindValue(due.dateTime().toString(Qt::ISODate));x.addBindValue(max.value());x.addBindValue(uid);if(title.text().trimmed().isEmpty()||!x.exec())QMessageBox::warning(this,"Error","Could not create assignment. "+x.lastError().text());else QMessageBox::information(this,"Created","Assignment created.");}
        });
        auto extension=[=](bool classWide){
            QDialog d(this);d.setWindowTitle(classWide?"Extend whole class":"Individual extension");QFormLayout f(&d);QComboBox a,s;QSqlQuery q;q.prepare("SELECT a.id,c.code||' — '||a.title FROM assignments a JOIN courses c ON c.id=a.course_id WHERE c.lecturer_id=?");q.addBindValue(uid);q.exec();while(q.next())a.addItem(q.value(1).toString(),q.value(0));if(!classWide){QSqlQuery q2("SELECT id,full_name FROM users WHERE role='student' AND active=1");while(q2.next())s.addItem(q2.value(1).toString(),q2.value(0));}
            QDateTimeEdit due(QDateTime::currentDateTime().addDays(2));due.setCalendarPopup(true);QLineEdit reason;f.addRow("Assignment",&a);if(!classWide)f.addRow("Student",&s);f.addRow("New deadline",&due);f.addRow("Reason",&reason);QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);if(d.exec()!=QDialog::Accepted)return;
            if(classWide){QSqlQuery x;x.prepare("UPDATE assignments SET class_extension=? WHERE id=? AND course_id IN(SELECT id FROM courses WHERE lecturer_id=?)");x.addBindValue(due.dateTime().toString(Qt::ISODate));x.addBindValue(a.currentData());x.addBindValue(uid);if(!x.exec()||x.numRowsAffected()==0)QMessageBox::warning(this,"Error","Could not extend class deadline. "+x.lastError().text());else QMessageBox::information(this,"Extension saved","The updated deadline will appear on students' assignment lists.");}
            else {QSqlQuery x;x.prepare("INSERT INTO individual_extensions(assignment_id,student_id,due_at,reason) VALUES(?,?,?,?) ON CONFLICT(assignment_id,student_id) DO UPDATE SET due_at=excluded.due_at,reason=excluded.reason");x.addBindValue(a.currentData());x.addBindValue(s.currentData());x.addBindValue(due.dateTime().toString(Qt::ISODate));x.addBindValue(reason.text());if(!x.exec())QMessageBox::warning(this,"Error",x.lastError().text());}
        };
        connect(individual,&QPushButton::clicked,this,[=](){extension(false);});connect(whole,&QPushButton::clicked,this,[=](){extension(true);});
        connect(mark,&QPushButton::clicked,this,[=](){
            int row=t->currentRow();if(row<0){QMessageBox::information(this,"Select submission","Select a submission first.");return;}int sid=t->item(row,0)->text().toInt();
            QSqlQuery check;check.prepare("SELECT s.status,a.max_mark,s.raw_mark FROM submissions s JOIN assignments a ON a.id=s.assignment_id JOIN courses c ON c.id=a.course_id WHERE s.id=? AND c.lecturer_id=?");check.addBindValue(sid);check.addBindValue(uid);if(!check.exec()||!check.next()){QMessageBox::warning(this,"Not allowed","This submission is not assigned to you.");return;}if(check.value(0).toString()=="Released"){QMessageBox::warning(this,"Locked","Released marks cannot be edited.");return;}
            bool ok=false;double raw=QInputDialog::getDouble(this,"Mark submission","Raw mark (before late penalty):",check.value(2).isNull()?0:check.value(2).toDouble(),0,check.value(1).toDouble(),2,&ok);if(!ok)return;QString feedback=QInputDialog::getMultiLineText(this,"Written feedback","Feedback:","",&ok);if(!ok)return;QString reason=QInputDialog::getText(this,"Audit reason","Reason for mark change:",QLineEdit::Normal,"Initial marking",&ok);if(!ok||reason.trimmed().isEmpty())return;
            QSqlQuery sub;
sub.prepare(
    "SELECT s.submitted_at, "
    "COALESCE(i.due_at, a.class_extension, a.due_at), "
    "a.max_mark "
    "FROM submissions s "
    "JOIN assignments a ON a.id = s.assignment_id "
    "LEFT JOIN individual_extensions i "
    "ON i.assignment_id = a.id "
    "AND i.student_id = s.student_id "
    "WHERE s.id = ?"
);

sub.addBindValue(sid);

if (!sub.exec() || !sub.next()) {
    QMessageBox::warning(
        this,
        "Deadline error",
        "Could not retrieve the submission deadline: "
            + sub.lastError().text()
    );
    return;
}

QString submittedText = sub.value(0).toString().trimmed();
QString dueText = sub.value(1).toString().trimmed();

QDateTime submitted =
    QDateTime::fromString(submittedText, Qt::ISODate);

QDateTime due =
    QDateTime::fromString(dueText, Qt::ISODate);

if (!submitted.isValid() || !due.isValid()) {
    QMessageBox::warning(
        this,
        "Invalid dates",
        QString("Could not calculate the late penalty.\n"
                "Submitted: %1\nDeadline: %2")
            .arg(submittedText, dueText)
    );
    return;
}

int days = 0;

if (submitted.isValid() && due.isValid()) {
    qint64 secondsLate = due.secsTo(submitted);

    if (secondsLate > 0) {
        days = 1 + static_cast<int>(
            (secondsLate - 1) / 86400
        );
    }
}

double maxMark = sub.value(2).toDouble();

lms::StandardLatePenalty penaltyPolicy;

double finalMark =
    penaltyPolicy.finalMark(raw, maxMark, days);
            QSqlQuery old;old.prepare("SELECT raw_mark FROM submissions WHERE id=?");old.addBindValue(sid);old.exec();old.next();QVariant oldMark=old.value(0);
            auto db = QSqlDatabase::database();
            if (!db.transaction()) { QMessageBox::warning(this,"Database error","Could not start mark transaction: "+db.lastError().text()); return; }
            QSqlQuery upd;upd.prepare("UPDATE submissions SET raw_mark=?,final_mark=?,feedback=?,marked_by=?,status='Marked' WHERE id=?");upd.addBindValue(raw);upd.addBindValue(finalMark);upd.addBindValue(feedback);upd.addBindValue(uid);upd.addBindValue(sid);
            if(!upd.exec()) { db.rollback(); QMessageBox::warning(this,"Error",upd.lastError().text()); return; }
            QSqlQuery audit;audit.prepare("INSERT INTO mark_audit(submission_id,actor_id,changed_at,old_mark,new_mark,reason) VALUES(?,?,?,?,?,?)");audit.addBindValue(sid);audit.addBindValue(uid);audit.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));audit.addBindValue(oldMark);audit.addBindValue(finalMark);audit.addBindValue(reason);
            if(!audit.exec()) { db.rollback(); QMessageBox::warning(this,"Audit error","Mark change was rolled back because its audit record could not be saved: "+audit.lastError().text()); return; }
            if(!db.commit()) { db.rollback(); QMessageBox::warning(this,"Database error","Could not commit mark and audit record: "+db.lastError().text()); return; }
            QMessageBox::information(this,"Marked",QString("Final mark after late penalty: %1 / %2").arg(finalMark).arg(sub.value(2).toDouble()));refresh();
        });
        tabs->addTab(w,"Teaching");refresh();
    }
