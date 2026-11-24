#pragma once

#include <QSqlDatabase>
#include <QString>

struct UserInfo {
    int id = -1;
    QString fullname;
    QString username;
    QString role;
};

class DatabaseManager {
public:
    static DatabaseManager &instance();

    bool open(const QString &path);
    void close();
    bool isOpen() const;
    QSqlDatabase database() const;
    QString lastError() const;
    QString databasePath() const;

    bool authenticate(const QString &username, const QString &password,
                      UserInfo *user, QString *error);
    bool logEvent(const QString &username, const QString &event);

    static bool probe(const QString &path, QString *error);

private:
    DatabaseManager() = default;
    bool initSchema();
    bool seed();

    QString m_connectionName = QStringLiteral("hospital_sqlite");
    QString m_lastError;
    QString m_path;
};
