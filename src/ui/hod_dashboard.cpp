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

void MainWindow::buildHod() {
        QVBoxLayout *l;auto *w=page("Moderation queue","Review marks in your department. Students cannot see marks until you release them.",&l);
        auto *bar=new QHBoxLayout;auto *moderate=new QPushButton("Moderate selected mark");auto *release=new QPushButton("Release selected mark");auto *auditBtn=new QPushButton("View mark audit");bar->addWidget(moderate);bar->addWidget(release);bar->addWidget(auditBtn);l->addLayout(bar);
        auto *t=table(l,{"ID","Course","Assignment","Student","Submitted","Status","Raw mark","Final mark","Feedback"});refreshSubmissions(t,true);
        auto allowed=[=](int sid){QSqlQuery q;q.prepare("SELECT s.status FROM submissions s JOIN assignments a ON a.id=s.assignment_id JOIN courses c ON c.id=a.course_id JOIN users me ON me.id=? WHERE s.id=? AND c.department_id=me.department_id");q.addBindValue(uid);q.addBindValue(sid);return q.exec()&&q.next()?q.value(0).toString():QString();};
        connect(moderate,&QPushButton::clicked,this,[=](){int row=t->currentRow();if(row<0)return;int sid=t->item(row,0)->text().toInt();QString st=allowed(sid);if(st!="Marked"&&st!="Moderated"){QMessageBox::warning(this,"Cannot moderate","Only marked submissions in your department can be moderated.");return;}bool ok=false;QString note=QInputDialog::getText(this,"Moderate mark","Moderation note / reason:",QLineEdit::Normal,"Reviewed by HoD",&ok);if(!ok||note.trimmed().isEmpty())return;QSqlQuery q;q.prepare("UPDATE submissions SET status='Moderated',moderated_by=?,feedback=CASE WHEN feedback='' THEN ? ELSE feedback||char(10)||? END WHERE id=?");q.addBindValue(uid);q.addBindValue("[Moderation] "+note);q.addBindValue("[Moderation] "+note);q.addBindValue(sid);if(!q.exec())QMessageBox::warning(this,"Error",q.lastError().text());refreshSubmissions(t,true);});
        connect(release,&QPushButton::clicked,this,[=](){int row=t->currentRow();if(row<0)return;int sid=t->item(row,0)->text().toInt();if(allowed(sid)!="Moderated"){QMessageBox::warning(this,"Cannot release","Moderate the submission first.");return;}if(QMessageBox::question(this,"Release mark","Release this mark and feedback to the student?")!=QMessageBox::Yes)return;QSqlQuery q;q.prepare("UPDATE submissions SET status='Released',released_at=? WHERE id=?");q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));q.addBindValue(sid);q.exec();refreshSubmissions(t,true);});
        connect(auditBtn,&QPushButton::clicked,this,[=](){
            QDialog d(this);d.setWindowTitle("Department mark change audit");d.resize(800,400);auto *v=new QVBoxLayout(&d);auto *at=table(v,{"ID","Submission","Actor","Changed at","Old mark","New mark","Reason"});
            QSqlQuery q;q.prepare("SELECT ma.id,ma.submission_id,u.username,ma.changed_at,COALESCE(ma.old_mark,''),ma.new_mark,ma.reason FROM mark_audit ma JOIN users u ON u.id=ma.actor_id JOIN submissions s ON s.id=ma.submission_id JOIN assignments a ON a.id=s.assignment_id JOIN courses c ON c.id=a.course_id JOIN users me ON me.id=? WHERE c.department_id=me.department_id ORDER BY ma.changed_at DESC");q.addBindValue(uid);
            at->setRowCount(0);if(q.exec()){while(q.next()){int r=at->rowCount();at->insertRow(r);for(int c=0;c<7;++c)at->setItem(r,c,new QTableWidgetItem(q.value(c).toString()));}}else QMessageBox::warning(this,"Audit error",q.lastError().text());d.exec();});
        tabs->addTab(w,"Moderation");
    }
