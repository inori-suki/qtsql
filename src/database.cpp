#include "database.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace {

QString passwordHash(const QString &password)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

struct SeedPatient {
    const char *idCard;
    const char *name;
    const char *sex;
    const char *dob;
    double height;
    double weight;
    const char *mobile;
};

int ageFromDob(const QString &dob)
{
    const QDate birth = QDate::fromString(dob, QStringLiteral("yyyy-MM-dd"));
    if (!birth.isValid()) {
        return 0;
    }
    const QDate today = QDate::currentDate();
    int age = today.year() - birth.year();
    if (today < birth.addYears(age)) {
        --age;
    }
    return age;
}

} // namespace

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

bool DatabaseManager::open(const QString &path)
{
    close();
    m_lastError.clear();

    if (path.trimmed().isEmpty()) {
        m_lastError = QStringLiteral("数据库路径为空");
        return false;
    }

    const QFileInfo info(path);
    if (!info.absoluteDir().exists()) {
        m_lastError = QStringLiteral("数据库目录不存在：%1").arg(info.absolutePath());
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        m_lastError = db.lastError().text();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
        return false;
    }

    m_path = path;
    QSqlQuery pragma(db);
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    pragma.exec(QStringLiteral("PRAGMA journal_mode = WAL"));
    pragma.finish();

    if (!initSchema()) {
        db.close();
        db = QSqlDatabase();
        close();
        return false;
    }
    if (!seed()) {
        db.close();
        db = QSqlDatabase();
        close();
        return false;
    }
    return true;
}

void DatabaseManager::close()
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return;
    }
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(m_connectionName);
    m_path.clear();
}

bool DatabaseManager::isOpen() const
{
    return QSqlDatabase::contains(m_connectionName)
        && QSqlDatabase::database(m_connectionName, false).isOpen();
}

QSqlDatabase DatabaseManager::database() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

QString DatabaseManager::databasePath() const
{
    return m_path;
}

bool DatabaseManager::initSchema()
{
    QSqlDatabase db = database();
    QSqlQuery query(db);

    const QStringList statements = {
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS department ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " name TEXT NOT NULL UNIQUE"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS doctor ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " employee_no TEXT NOT NULL UNIQUE,"
            " name TEXT NOT NULL,"
            " department_id INTEGER NOT NULL,"
            " title TEXT,"
            " FOREIGN KEY(department_id) REFERENCES department(id)"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS patient ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " name TEXT NOT NULL,"
            " id_card TEXT NOT NULL UNIQUE,"
            " sex TEXT NOT NULL CHECK(sex IN ('男', '女')),"
            " age INTEGER,"
            " dob TEXT NOT NULL,"
            " height REAL,"
            " weight REAL,"
            " mobile TEXT NOT NULL,"
            " created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime'))"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS user ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " fullname TEXT NOT NULL,"
            " username TEXT NOT NULL UNIQUE,"
            " password_hash TEXT NOT NULL,"
            " role TEXT NOT NULL DEFAULT 'doctor'"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS history ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " user_id INTEGER,"
            " username TEXT NOT NULL,"
            " event TEXT NOT NULL,"
            " created_at TEXT NOT NULL DEFAULT (datetime('now', 'localtime'))"
            ")"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_patient_name ON patient(name)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_patient_mobile ON patient(mobile)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_history_time ON history(created_at)"),
    };

    for (const QString &sql : statements) {
        if (!query.exec(sql)) {
            m_lastError = QStringLiteral("建表失败：%1").arg(query.lastError().text());
            return false;
        }
    }
    return true;
}

bool DatabaseManager::seed()
{
    QSqlDatabase db = database();
    QSqlQuery countQuery(db);
    if (!countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM user")) || !countQuery.next()) {
        m_lastError = QStringLiteral("检查初始化数据失败：%1").arg(countQuery.lastError().text());
        return false;
    }
    const int existingUsers = countQuery.value(0).toInt();
    countQuery.finish();
    if (existingUsers > 0) {
        return true;
    }

    if (!db.transaction()) {
        m_lastError = QStringLiteral("无法启动初始化事务");
        return false;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("INSERT INTO department(name) VALUES(?)"));
    const QStringList departments = {
        QStringLiteral("内科"), QStringLiteral("外科"), QStringLiteral("儿科"),
        QStringLiteral("骨科"), QStringLiteral("妇产科")
    };
    for (const QString &name : departments) {
        query.bindValue(0, name);
        if (!query.exec()) {
            m_lastError = QStringLiteral("初始化科室失败：%1").arg(query.lastError().text());
            db.rollback();
            return false;
        }
    }

    struct SeedDoctor {
        const char *employeeNo;
        const char *name;
        int departmentId;
        const char *title;
    };
    const SeedDoctor doctors[] = {
        {"D001", "张伟", 1, "主任医师"},
        {"D002", "李娜", 2, "副主任医师"},
        {"D003", "王强", 3, "主治医师"},
        {"D004", "赵敏", 4, "主任医师"},
        {"D005", "陈静", 5, "副主任医师"},
    };
    query.prepare(QStringLiteral(
        "INSERT INTO doctor(employee_no, name, department_id, title) VALUES(?, ?, ?, ?)"));
    for (const SeedDoctor &doctor : doctors) {
        query.bindValue(0, QString::fromUtf8(doctor.employeeNo));
        query.bindValue(1, QString::fromUtf8(doctor.name));
        query.bindValue(2, doctor.departmentId);
        query.bindValue(3, QString::fromUtf8(doctor.title));
        if (!query.exec()) {
            m_lastError = QStringLiteral("初始化医生失败：%1").arg(query.lastError().text());
            db.rollback();
            return false;
        }
    }

    query.prepare(QStringLiteral(
        "INSERT INTO user(fullname, username, password_hash, role) VALUES(?, ?, ?, ?)"));
    const QList<QVariantList> users = {
        {QStringLiteral("系统管理员"), QStringLiteral("admin"),
         passwordHash(QStringLiteral("123456")), QStringLiteral("管理员")},
        {QStringLiteral("李医生"), QStringLiteral("doctor"),
         passwordHash(QStringLiteral("123456")), QStringLiteral("医生")},
    };
    for (const QVariantList &user : users) {
        for (int i = 0; i < user.size(); ++i) {
            query.bindValue(i, user.at(i));
        }
        if (!query.exec()) {
            m_lastError = QStringLiteral("初始化用户失败：%1").arg(query.lastError().text());
            db.rollback();
            return false;
        }
    }

    const SeedPatient patients[] = {
        {"440101199001010011", "陈志强", "男", "1990-01-01", 175.0, 70.0, "13800138001"},
        {"440101199203150022", "李婉婷", "女", "1992-03-15", 162.0, 52.0, "13900139002"},
        {"440101198807220033", "张伟明", "男", "1988-07-22", 178.0, 75.0, "13700137003"},
        {"440101199511300044", "王丽娟", "女", "1995-11-30", 158.0, 48.0, "13600136004"},
        {"440101200002180055", "刘浩然", "男", "2000-02-18", 180.0, 72.0, "13500135005"},
        {"440101199709090066", "赵敏", "女", "1997-09-09", 165.0, 55.0, "13400134006"},
        {"440101198512120077", "周建国", "男", "1985-12-12", 172.0, 68.0, "13300133007"},
        {"440101200105250088", "吴思雨", "女", "2001-05-25", 160.0, 50.0, "13200132008"},
        {"440101199308080099", "郑凯", "男", "1993-08-08", 176.0, 74.0, "13100131009"},
        {"44010119991201001X", "孙悦", "女", "1999-12-01", 163.0, 51.0, "13000130010"},
    };
    query.prepare(QStringLiteral(
        "INSERT INTO patient(id_card, name, sex, dob, height, weight, mobile, age, created_at)"
        " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    for (int i = 0; i < int(sizeof(patients) / sizeof(patients[0])); ++i) {
        const SeedPatient &p = patients[i];
        query.bindValue(0, QString::fromUtf8(p.idCard));
        query.bindValue(1, QString::fromUtf8(p.name));
        query.bindValue(2, QString::fromUtf8(p.sex));
        query.bindValue(3, QString::fromUtf8(p.dob));
        query.bindValue(4, p.height);
        query.bindValue(5, p.weight);
        query.bindValue(6, QString::fromUtf8(p.mobile));
        query.bindValue(7, ageFromDob(QString::fromUtf8(p.dob)));
        query.bindValue(8, QStringLiteral("2026-10-%1 09:%2:00")
                               .arg(i + 10, 2, 10, QLatin1Char('0'))
                               .arg(10 + i, 2, 10, QLatin1Char('0')));
        if (!query.exec()) {
            m_lastError = QStringLiteral("初始化患者失败：%1").arg(query.lastError().text());
            db.rollback();
            return false;
        }
    }

    query.exec(QStringLiteral(
        "INSERT INTO history(username, event) VALUES('system', '数据库初始化完成，预置10条患者档案')"));
    query.finish();

    if (!db.commit()) {
        m_lastError = QStringLiteral("初始化数据提交失败：%1").arg(db.lastError().text());
        db.rollback();
        return false;
    }
    return true;
}

bool DatabaseManager::authenticate(const QString &username, const QString &password,
                                   UserInfo *user, QString *error)
{
    if (!isOpen()) {
        if (error) {
            *error = QStringLiteral("数据库未连接");
        }
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral(
        "SELECT id, fullname, username, password_hash, role FROM user WHERE username = ?"));
    query.addBindValue(username.trimmed());
    if (!query.exec()) {
        if (error) {
            *error = QStringLiteral("用户查询失败：%1").arg(query.lastError().text());
        }
        return false;
    }
    if (!query.next()) {
        if (error) {
            *error = QStringLiteral("用户名或密码错误");
        }
        return false;
    }

    const QString stored = query.value(3).toString();
    const QString actual = passwordHash(password);
    if (stored.compare(actual, Qt::CaseInsensitive) != 0) {
        if (error) {
            *error = QStringLiteral("用户名或密码错误");
        }
        return false;
    }

    if (user) {
        user->id = query.value(0).toInt();
        user->fullname = query.value(1).toString();
        user->username = query.value(2).toString();
        user->role = query.value(4).toString();
    }
    return true;
}

bool DatabaseManager::logEvent(const QString &username, const QString &event)
{
    if (!isOpen()) {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral(
        "INSERT INTO history(username, event) VALUES(?, ?)"));
    query.addBindValue(username.trimmed().isEmpty() ? QStringLiteral("anonymous") : username.trimmed());
    query.addBindValue(event);
    return query.exec();
}

bool DatabaseManager::probe(const QString &path, QString *error)
{
    const QString connection = QStringLiteral("probe_%1").arg(
        QUuid::createUuid().toString(QUuid::WithoutBraces));
    bool ok = false;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        db.setDatabaseName(path);
        if (db.open()) {
            ok = true;
            db.close();
        } else if (error) {
            *error = db.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(connection);
    return ok;
}
