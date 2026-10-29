#include "lms/common.hpp"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDir>
#include <QRandomGenerator>
#include <QDebug>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <array>

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QTabWidget>
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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDate>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QFileInfo>
#include <QDir>
#include <QGroupBox>
#include <QStatusBar>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QMessageBox>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "lms/domain.hpp"
#include <array>

QString dbError() { return QSqlDatabase::database().lastError().text(); }
static bool execSql(const QString &sql) {
    QSqlQuery q;
    if (!q.exec(sql)) { qWarning() << q.lastError().text() << sql; return false; }
    return true;
}
QByteArray randomSalt() {
    QByteArray salt(16, Qt::Uninitialized);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()), salt.size()) != 1)
        for (int i=0;i<salt.size();++i) salt[i]=char(QRandomGenerator::global()->generate() & 0xff);
    return salt;
}
QByteArray hashPassword(const QString &password, const QByteArray &salt) {
    QByteArray out(32, Qt::Uninitialized);
    const QByteArray pass = password.toUtf8();
    // OpenSSL scrypt: memory-hard password derivation; parameters are intentionally moderate for school PCs.
    if (EVP_PBE_scrypt(pass.constData(), size_t(pass.size()),
                       reinterpret_cast<const unsigned char*>(salt.constData()), size_t(salt.size()),
                       16384, 8, 1, 32 * 1024 * 1024,
                       reinterpret_cast<unsigned char*>(out.data()), size_t(out.size())) != 1)
        return {};
    return out.toHex();
}
bool setPassword(int userId, const QString &password) {
    QByteArray salt = randomSalt();
    QByteArray hash = hashPassword(password, salt);
    QSqlQuery q;
    q.prepare("UPDATE users SET salt=?, password_hash=? WHERE id=?");
    q.addBindValue(salt.toHex()); q.addBindValue(QString::fromLatin1(hash)); q.addBindValue(userId);
    return q.exec();
}
bool verifyPassword(const QString &password, const QString &saltHex, const QString &hashHex) {
    QByteArray got = hashPassword(password, QByteArray::fromHex(saltHex.toLatin1()));
    return !got.isEmpty() && got == hashHex.toLatin1();
}
QString roleName(const QString &r) {
    if (r=="admin") return "Administrator";
    if (r=="lecturer") return "Lecturer";
    if (r=="hod") return "Head of Department";
    return "Student";
}
static void seedUser(const QString &username, const QString &name, const QString &role, int dept=1) {
    QSqlQuery q; q.prepare("INSERT OR IGNORE INTO users(username,full_name,role,department_id,salt,password_hash) VALUES(?,?,?,?,?,?)");
    QByteArray salt=randomSalt();
    q.addBindValue(username); q.addBindValue(name); q.addBindValue(role); q.addBindValue(dept);
    q.addBindValue(salt.toHex()); q.addBindValue(QString::fromLatin1(hashPassword("ChangeMe123!",salt)));
    q.exec();
}
bool initDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::current().filePath("moderated_lms.sqlite"));
    if (!db.open()) return false;
    const QStringList schema = {
      "PRAGMA foreign_keys=ON",
      "CREATE TABLE IF NOT EXISTS departments(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE)",
      "CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE, full_name TEXT NOT NULL, role TEXT NOT NULL CHECK(role IN('admin','lecturer','hod','student')), department_id INTEGER REFERENCES departments(id), salt TEXT NOT NULL, password_hash TEXT NOT NULL, active INTEGER NOT NULL DEFAULT 1)",
      "CREATE TABLE IF NOT EXISTS courses(id INTEGER PRIMARY KEY, code TEXT NOT NULL UNIQUE, title TEXT NOT NULL, department_id INTEGER NOT NULL REFERENCES departments(id), lecturer_id INTEGER REFERENCES users(id))",
      "CREATE TABLE IF NOT EXISTS enrolments(student_id INTEGER NOT NULL REFERENCES users(id), course_id INTEGER NOT NULL REFERENCES courses(id), PRIMARY KEY(student_id,course_id))",
      "CREATE TABLE IF NOT EXISTS assignments(id INTEGER PRIMARY KEY, course_id INTEGER NOT NULL REFERENCES courses(id), title TEXT NOT NULL, description TEXT NOT NULL DEFAULT '', due_at TEXT NOT NULL, max_mark REAL NOT NULL CHECK(max_mark>0), created_by INTEGER NOT NULL REFERENCES users(id), class_extension TEXT)",
      "CREATE TABLE IF NOT EXISTS individual_extensions(assignment_id INTEGER NOT NULL REFERENCES assignments(id), student_id INTEGER NOT NULL REFERENCES users(id), due_at TEXT NOT NULL, reason TEXT NOT NULL, PRIMARY KEY(assignment_id,student_id))",
      "CREATE TABLE IF NOT EXISTS submissions(id INTEGER PRIMARY KEY, assignment_id INTEGER NOT NULL REFERENCES assignments(id), student_id INTEGER NOT NULL REFERENCES users(id), content TEXT NOT NULL, submitted_at TEXT NOT NULL, status TEXT NOT NULL DEFAULT 'Submitted', raw_mark REAL, final_mark REAL, feedback TEXT NOT NULL DEFAULT '', marked_by INTEGER REFERENCES users(id), moderated_by INTEGER REFERENCES users(id), released_at TEXT, UNIQUE(assignment_id,student_id))",
      "CREATE TABLE IF NOT EXISTS mark_audit(id INTEGER PRIMARY KEY, submission_id INTEGER NOT NULL REFERENCES submissions(id), actor_id INTEGER NOT NULL REFERENCES users(id), changed_at TEXT NOT NULL, old_mark REAL, new_mark REAL NOT NULL, reason TEXT NOT NULL)",
      "CREATE INDEX IF NOT EXISTS idx_submissions_assignment ON submissions(assignment_id)",
      "CREATE INDEX IF NOT EXISTS idx_courses_department ON courses(department_id)"
    };
    for (const auto &s:schema) if(!execSql(s)) return false;
    execSql("INSERT OR IGNORE INTO departments(id,name) VALUES(1,'General Studies')");
    QSqlQuery count("SELECT COUNT(*) FROM users"); count.next();
    if (count.value(0).toInt()==0) {
        seedUser("admin","System Administrator","admin");
        seedUser("lecturer","Demo Lecturer","lecturer");
        seedUser("hod","Demo Head of Department","hod");
        seedUser("student","Demo Student","student");
        QSqlQuery q; q.prepare("INSERT INTO courses(code,title,department_id,lecturer_id) VALUES('CS101','Introduction to Computing',1,(SELECT id FROM users WHERE username='lecturer'))"); q.exec();
        q.exec("INSERT INTO enrolments(student_id,course_id) SELECT u.id,c.id FROM users u,courses c WHERE u.username='student' AND c.code='CS101'");
        q.exec("INSERT INTO assignments(course_id,title,description,due_at,max_mark,created_by) SELECT c.id,'Assignment 1','Write a short introduction to computing.',datetime('now','+7 days'),100,u.id FROM courses c,users u WHERE c.code='CS101' AND u.username='lecturer'");
    }
    return true;
}

