#pragma once
#include <QMainWindow>
#include <QString>
#include <QStringList>
class QTabWidget;
class QWidget;
class QVBoxLayout;
class QTableWidget;

class MainWindow : public QMainWindow {
    int uid;
    QString role, fullName, uname;
    QTabWidget *tabs;
public:
    MainWindow(int id, QString r, QString name, QString user);
private:
    QWidget *page(const QString &title, const QString &subtitle, QVBoxLayout **out=nullptr);
    QTableWidget *table(QVBoxLayout *layout, const QStringList &headers);
    void fill(QTableWidget *table, const QString &sql, int columns);
    void refreshSubmissions(QTableWidget *table, bool departmentOnly=false, bool studentOnly=false);
    void buildAdmin();
    void buildLecturer();
    void buildHod();
    void buildStudent();
};
