#pragma once
#include <QString>
#include <QByteArray>

QString dbError();
QByteArray randomSalt();
QByteArray hashPassword(const QString &password, const QByteArray &salt);
bool setPassword(int userId, const QString &password);
bool verifyPassword(const QString &password, const QString &saltHex, const QString &hashHex);
QString roleName(const QString &role);
bool initDatabase();
