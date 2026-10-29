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

void MainWindow::buildAdmin() {
        QVBoxLayout *l; auto *w=page("Administration","Manage, edit, filter and export LMS records.",&l);
        auto *actions=new QHBoxLayout;
        auto *users=new QPushButton("Create user");
        auto *dept=new QPushButton("Add department");
        auto *course=new QPushButton("Create course");
        auto *assign=new QPushButton("Assign lecturer");
        auto *enrol=new QPushButton("Enrol student");
        auto *edit=new QPushButton("Edit selected");
        auto *remove=new QPushButton("Remove selected enrolment");
        auto *exportButton=new QPushButton("Export table to CSV");
        for(auto *b:{users,dept,course,assign,enrol,edit,remove,exportButton}) actions->addWidget(b);
        l->addLayout(actions);

        auto *filters=new QHBoxLayout;
        filters->addWidget(new QLabel("View:"));
        auto *view=new QComboBox;
        view->addItem("All users","users");
        view->addItem("Students","students");
        view->addItem("Lecturers","lecturers");
        view->addItem("Heads of Department","hods");
        view->addItem("Administrators","admins");
        view->addItem("Departments","departments");
        view->addItem("Courses","courses");
        view->addItem("Enrolments","enrolments");
        view->addItem("Assignments","assignments");
        filters->addWidget(view);
        auto *search=new QLineEdit; search->setPlaceholderText("Search visible records...");
        filters->addWidget(search,1); l->addLayout(filters);

        auto *t=table(l,{"ID","Username","Full name","Role","Department","Active"});
        auto load=[=]() {
            const QString key=view->currentData().toString();
            QString sql; QStringList headers;
            if(key=="users"||key=="students"||key=="lecturers"||key=="hods"||key=="admins") {
                sql="SELECT u.id,u.username,u.full_name,u.role,COALESCE(d.name,''),u.active FROM users u LEFT JOIN departments d ON d.id=u.department_id";
                if(key=="students") sql+=" WHERE u.role='student'";
                else if(key=="lecturers") sql+=" WHERE u.role='lecturer'";
                else if(key=="hods") sql+=" WHERE u.role='hod'";
                else if(key=="admins") sql+=" WHERE u.role='admin'";
                sql+=" ORDER BY u.id";
                headers={"ID","Username","Full name","Role","Department","Active"};
            } else if(key=="departments") {
                sql="SELECT id,name FROM departments ORDER BY name"; headers={"ID","Department"};
            } else if(key=="courses") {
                sql="SELECT c.id,c.code,c.title,d.name,COALESCE(u.full_name,'Unassigned') FROM courses c JOIN departments d ON d.id=c.department_id LEFT JOIN users u ON u.id=c.lecturer_id ORDER BY c.code";
                headers={"ID","Course code","Title","Department","Lecturer"};
            } else if(key=="enrolments") {
                sql="SELECT e.student_id,u.full_name,c.code,c.title FROM enrolments e JOIN users u ON u.id=e.student_id JOIN courses c ON c.id=e.course_id ORDER BY u.full_name,c.code";
                headers={"Student ID","Student","Course code","Course"};
            } else {
                sql="SELECT a.id,c.code,a.title,a.due_at,a.max_mark,COALESCE(a.class_extension,'') FROM assignments a JOIN courses c ON c.id=a.course_id ORDER BY a.due_at";
                headers={"ID","Course","Assignment","Deadline","Maximum mark","Class extension"};
            }
            t->setRowCount(0); t->setColumnCount(headers.size()); t->setHorizontalHeaderLabels(headers);
            QSqlQuery q(sql);
            while(q.next()) {
                int row=t->rowCount(); t->insertRow(row);
                for(int c=0;c<headers.size();++c) t->setItem(row,c,new QTableWidgetItem(q.value(c).toString()));
            }
            QString needle=search->text().trimmed();
            for(int row=0;row<t->rowCount();++row) {
                bool match=needle.isEmpty();
                for(int col=0;col<t->columnCount()&&!match;++col)
                    if(t->item(row,col)&&t->item(row,col)->text().contains(needle,Qt::CaseInsensitive)) match=true;
                t->setRowHidden(row,!match);
            }
        };
        auto refresh=[=](){load();};
        connect(view,&QComboBox::currentIndexChanged,this,[=](int){load();});
        connect(search,&QLineEdit::textChanged,this,[=](const QString&){load();});

        connect(users,&QPushButton::clicked,this,[=](){
            QDialog d(this);d.setWindowTitle("Create user");QFormLayout f(&d);
            QLineEdit username,name,password;password.setText("ChangeMe123!");
            QComboBox r;r.addItem("Student","student");r.addItem("Lecturer","lecturer");r.addItem("Head of Department","hod");r.addItem("Administrator","admin");
            QComboBox dep;QSqlQuery q("SELECT id,name FROM departments ORDER BY name");while(q.next())dep.addItem(q.value(1).toString(),q.value(0));
            f.addRow("Username",&username);f.addRow("Full name",&name);f.addRow("Temporary password",&password);f.addRow("Role",&r);f.addRow("Department",&dep);
            QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
            if(d.exec()!=QDialog::Accepted)return;
            if(username.text().trimmed().isEmpty()||name.text().trimmed().isEmpty()||password.text().size()<10||dep.currentIndex()<0){QMessageBox::warning(this,"Invalid input","Enter a username, full name, department and password of at least 10 characters.");return;}
            QSqlQuery ins;ins.prepare("INSERT INTO users(username,full_name,role,department_id,salt,password_hash) VALUES(?,?,?,?,?,?)");
            auto salt=randomSalt();ins.addBindValue(username.text().trimmed());ins.addBindValue(name.text().trimmed());ins.addBindValue(r.currentData());ins.addBindValue(dep.currentData());ins.addBindValue(salt.toHex());ins.addBindValue(QString::fromLatin1(hashPassword(password.text(),salt)));
            if(!ins.exec())QMessageBox::warning(this,"Could not create user",ins.lastError().text());else load();
        });
        connect(dept,&QPushButton::clicked,this,[=](){
            bool ok=false;QString n=QInputDialog::getText(this,"Add department","Department name:",QLineEdit::Normal,"",&ok);
            if(ok&&!n.trimmed().isEmpty()){QSqlQuery q;q.prepare("INSERT INTO departments(name) VALUES(?)");q.addBindValue(n.trimmed());if(!q.exec())QMessageBox::warning(this,"Error",q.lastError().text());load();}
        });
        connect(course,&QPushButton::clicked,this,[=](){
            QDialog d(this);d.setWindowTitle("Create course");QFormLayout f(&d);QLineEdit code,title;QComboBox dep;
            QSqlQuery q("SELECT id,name FROM departments ORDER BY name");while(q.next())dep.addItem(q.value(1).toString(),q.value(0));
            f.addRow("Course code",&code);f.addRow("Title",&title);f.addRow("Department",&dep);
            QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
            if(d.exec()!=QDialog::Accepted)return;
            if(code.text().trimmed().isEmpty()||title.text().trimmed().isEmpty()||dep.currentIndex()<0){QMessageBox::warning(this,"Invalid input","Course code, title and department are required.");return;}
            QSqlQuery x;x.prepare("INSERT INTO courses(code,title,department_id) VALUES(?,?,?)");x.addBindValue(code.text().trimmed());x.addBindValue(title.text().trimmed());x.addBindValue(dep.currentData());if(!x.exec())QMessageBox::warning(this,"Error",x.lastError().text());load();
        });
        connect(assign,&QPushButton::clicked,this,[=](){
            QDialog d(this);QFormLayout f(&d);QComboBox c,lec;
            QSqlQuery q("SELECT id,code||' — '||title FROM courses ORDER BY code");while(q.next())c.addItem(q.value(1).toString(),q.value(0));
            QSqlQuery q2("SELECT id,full_name FROM users WHERE role='lecturer' AND active=1 ORDER BY full_name");while(q2.next())lec.addItem(q2.value(1).toString(),q2.value(0));
            f.addRow("Course",&c);f.addRow("Lecturer",&lec);QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
            if(d.exec()==QDialog::Accepted&&c.currentIndex()>=0&&lec.currentIndex()>=0){QSqlQuery x;x.prepare("UPDATE courses SET lecturer_id=? WHERE id=?");x.addBindValue(lec.currentData());x.addBindValue(c.currentData());if(!x.exec())QMessageBox::warning(this,"Error",x.lastError().text());load();}
        });
        connect(enrol,&QPushButton::clicked,this,[=](){
            QDialog d(this);QFormLayout f(&d);QComboBox st,c;
            QSqlQuery q("SELECT id,full_name FROM users WHERE role='student' AND active=1 ORDER BY full_name");while(q.next())st.addItem(q.value(1).toString(),q.value(0));
            QSqlQuery q2("SELECT id,code||' — '||title FROM courses ORDER BY code");while(q2.next())c.addItem(q2.value(1).toString(),q2.value(0));
            f.addRow("Student",&st);f.addRow("Course",&c);QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
            if(d.exec()==QDialog::Accepted&&st.currentIndex()>=0&&c.currentIndex()>=0){QSqlQuery x;x.prepare("INSERT OR IGNORE INTO enrolments(student_id,course_id) VALUES(?,?)");x.addBindValue(st.currentData());x.addBindValue(c.currentData());if(!x.exec())QMessageBox::warning(this,"Error",x.lastError().text());load();}
        });
        connect(edit,&QPushButton::clicked,this,[=](){
            int row=t->currentRow();if(row<0){QMessageBox::information(this,"Select record","Select a record in the table first.");return;}
            QString key=view->currentData().toString();int id=t->item(row,0)->text().toInt();
            if(key=="users"||key=="students"||key=="lecturers"||key=="hods"||key=="admins"){
                QSqlQuery get;get.prepare("SELECT username,full_name,role,department_id,active FROM users WHERE id=?");get.addBindValue(id);if(!get.exec()||!get.next())return;
                QDialog d(this);d.setWindowTitle("Edit user");QFormLayout f(&d);QLineEdit username(get.value(0).toString()),name(get.value(1).toString()),password;
                QComboBox r;r.addItem("Student","student");r.addItem("Lecturer","lecturer");r.addItem("Head of Department","hod");r.addItem("Administrator","admin");r.setCurrentIndex(std::max(0,r.findData(get.value(2))));
                QComboBox dep;QSqlQuery dq("SELECT id,name FROM departments ORDER BY name");while(dq.next())dep.addItem(dq.value(1).toString(),dq.value(0));dep.setCurrentIndex(dep.findData(get.value(3)));
                QComboBox active;active.addItem("Active",1);active.addItem("Inactive",0);active.setCurrentIndex(active.findData(get.value(4)));
                f.addRow("Username",&username);f.addRow("Full name",&name);f.addRow("Role",&r);f.addRow("Department",&dep);f.addRow("Account status",&active);password.setPlaceholderText("Leave blank to keep current password");f.addRow("Reset password (optional)",&password);
                QDialogButtonBox b(QDialogButtonBox::Save|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
                if(d.exec()!=QDialog::Accepted)return;
                if(username.text().trimmed().isEmpty()||name.text().trimmed().isEmpty()||dep.currentIndex()<0||(!password.text().isEmpty()&&password.text().size()<10)){QMessageBox::warning(this,"Invalid input","Username/name and department are required; a new password must be at least 10 characters.");return;}
                QSqlQuery up;up.prepare("UPDATE users SET username=?,full_name=?,role=?,department_id=?,active=? WHERE id=?");up.addBindValue(username.text().trimmed());up.addBindValue(name.text().trimmed());up.addBindValue(r.currentData());up.addBindValue(dep.currentData());up.addBindValue(active.currentData());up.addBindValue(id);
                if(!up.exec()){QMessageBox::warning(this,"Update failed",up.lastError().text());return;}
                if(!password.text().isEmpty()&&!setPassword(id,password.text())){QMessageBox::warning(this,"Password update failed",dbError());return;}load();
            } else if(key=="departments"){
                QSqlQuery get;get.prepare("SELECT name FROM departments WHERE id=?");get.addBindValue(id);get.exec();if(!get.next())return;
                bool ok=false;QString name=QInputDialog::getText(this,"Edit department","Department name:",QLineEdit::Normal,get.value(0).toString(),&ok);
                if(ok&&!name.trimmed().isEmpty()){QSqlQuery up;up.prepare("UPDATE departments SET name=? WHERE id=?");up.addBindValue(name.trimmed());up.addBindValue(id);if(!up.exec())QMessageBox::warning(this,"Update failed",up.lastError().text());load();}
            } else if(key=="courses"){
                QSqlQuery get;get.prepare("SELECT code,title,department_id,lecturer_id FROM courses WHERE id=?");get.addBindValue(id);get.exec();if(!get.next())return;
                QDialog d(this);d.setWindowTitle("Edit course");QFormLayout f(&d);QLineEdit code(get.value(0).toString()),title(get.value(1).toString());QComboBox dep,lec;
                QSqlQuery dq("SELECT id,name FROM departments ORDER BY name");while(dq.next())dep.addItem(dq.value(1).toString(),dq.value(0));dep.setCurrentIndex(dep.findData(get.value(2)));
                lec.addItem("Unassigned",QVariant());QSqlQuery lq("SELECT id,full_name FROM users WHERE role='lecturer' AND active=1 ORDER BY full_name");while(lq.next())lec.addItem(lq.value(1).toString(),lq.value(0));lec.setCurrentIndex(lec.findData(get.value(3)));
                f.addRow("Course code",&code);f.addRow("Title",&title);f.addRow("Department",&dep);f.addRow("Lecturer",&lec);QDialogButtonBox b(QDialogButtonBox::Save|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
                if(d.exec()!=QDialog::Accepted)return;
                QSqlQuery up;up.prepare("UPDATE courses SET code=?,title=?,department_id=?,lecturer_id=? WHERE id=?");up.addBindValue(code.text().trimmed());up.addBindValue(title.text().trimmed());up.addBindValue(dep.currentData());up.addBindValue(lec.currentData());up.addBindValue(id);if(code.text().trimmed().isEmpty()||title.text().trimmed().isEmpty()||!up.exec())QMessageBox::warning(this,"Update failed",up.lastError().text());load();
            } else if(key=="assignments"){
                QSqlQuery get;get.prepare("SELECT title,description,due_at,max_mark FROM assignments WHERE id=?");get.addBindValue(id);get.exec();if(!get.next())return;
                QDialog d(this);d.setWindowTitle("Edit assignment");QFormLayout f(&d);QLineEdit title(get.value(0).toString());QTextEdit desc;desc.setPlainText(get.value(1).toString());QDateTimeEdit due(QDateTime::fromString(get.value(2).toString(),Qt::ISODate));due.setCalendarPopup(true);QDoubleSpinBox max;max.setRange(1,10000);max.setValue(get.value(3).toDouble());
                f.addRow("Title",&title);f.addRow("Description",&desc);f.addRow("Deadline",&due);f.addRow("Maximum mark",&max);QDialogButtonBox b(QDialogButtonBox::Save|QDialogButtonBox::Cancel);f.addRow(&b);connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);
                if(d.exec()!=QDialog::Accepted)return;QSqlQuery up;up.prepare("UPDATE assignments SET title=?,description=?,due_at=?,max_mark=? WHERE id=?");up.addBindValue(title.text().trimmed());up.addBindValue(desc.toPlainText());up.addBindValue(due.dateTime().toString(Qt::ISODate));up.addBindValue(max.value());up.addBindValue(id);if(title.text().trimmed().isEmpty()||!up.exec())QMessageBox::warning(this,"Update failed",up.lastError().text());load();
            } else {
                QMessageBox::information(this,"Edit not available","For enrolments, use Remove selected enrolment and Enrol student to correct the relationship.");
            }
        });
        connect(remove,&QPushButton::clicked,this,[=](){
            if(view->currentData().toString()!="enrolments"){QMessageBox::information(this,"Choose enrolments","Select Enrolments in the View filter first.");return;}
            int row=t->currentRow();if(row<0)return;
            if(QMessageBox::question(this,"Remove enrolment","Remove this student from this course? Existing submission history will remain in the database.")!=QMessageBox::Yes)return;
            QSqlQuery q;q.prepare("DELETE FROM enrolments WHERE student_id=? AND course_id=(SELECT id FROM courses WHERE code=?)");q.addBindValue(t->item(row,0)->text().toInt());q.addBindValue(t->item(row,2)->text());if(!q.exec())QMessageBox::warning(this,"Remove failed",q.lastError().text());load();
        });
        connect(exportButton,&QPushButton::clicked,this,[=](){
            QString path=QFileDialog::getSaveFileName(this,"Export visible dashboard table",QDir::homePath()+"/lms_dashboard_export.csv","CSV spreadsheet (*.csv)");
            if(path.isEmpty())return;if(!path.endsWith(".csv",Qt::CaseInsensitive))path+=".csv";
            QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::warning(this,"Export failed",file.errorString());return;}
            QTextStream out(&file);auto csv=[](QString v){v.replace("\"","\"\"");return QString("\"")+v+QString("\"");};
            QStringList header;for(int c=0;c<t->columnCount();++c)header<<csv(t->horizontalHeaderItem(c)->text());out<<header.join(",")<<"\n";
            int count=0;for(int row=0;row<t->rowCount();++row){if(t->isRowHidden(row))continue;QStringList fields;for(int c=0;c<t->columnCount();++c)fields<<csv(t->item(row,c)?t->item(row,c)->text():"");out<<fields.join(",")<<"\n";++count;}
            file.close();QMessageBox::information(this,"Export complete",QString("Exported %1 visible rows to:\n%2\nOpen the CSV file in LibreOffice Calc or Excel.").arg(count).arg(path));
        });
        tabs->addTab(w,"Administration");load();
    }
