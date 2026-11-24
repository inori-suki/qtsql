#include "selftest.h"

#include "database.h"
#include "patient.h"
#include "patientmodel.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QStringConverter>
#include <QTextStream>
#include <cstdio>

namespace {

void removeDatabaseFiles(const QString &path)
{
    QFile::remove(path);
    QFile::remove(path + QStringLiteral("-wal"));
    QFile::remove(path + QStringLiteral("-shm"));
}

} // namespace

int runSelfTests(const QString &databasePath)
{
    removeDatabaseFiles(databasePath);
    QStringList results;
    int failures = 0;

    auto check = [&](bool ok, const QString &name, const QString &detail = QString()) {
        QString line = QStringLiteral("%1 %2").arg(ok ? QStringLiteral("[PASS]") : QStringLiteral("[FAIL]"), name);
        if (!detail.isEmpty()) {
            line += QStringLiteral("  ::  ") + detail;
        }
        results << line;
        if (!ok) {
            ++failures;
        }
    };

    if (!DatabaseManager::instance().open(databasePath)) {
        results << QStringLiteral("[FAIL] 数据库初始化失败  ::  ") + DatabaseManager::instance().lastError();
        failures++;
    } else {
        PatientModel model;
        check(model.rowCount() >= 10, QStringLiteral("初始数据加载"),
              QStringLiteral("patient 表记录数 = %1").arg(model.rowCount()));

        const Patient existing = model.patientAt(0);
        Patient duplicate = existing;
        duplicate.id = -1;
        duplicate.name = QStringLiteral("重复测试");
        QString error;
        const bool duplicateInserted = model.insertPatient(duplicate, &error);
        check(!duplicateInserted && error.contains(QStringLiteral("已存在")),
              QStringLiteral("重复身份证号被拒绝"), error);

        Patient missingName;
        missingName.idCard = QStringLiteral("440101200001010099");
        missingName.sex = QStringLiteral("男");
        missingName.dob = QStringLiteral("2000-01-01");
        missingName.mobile = QStringLiteral("13800000000");
        missingName.height = 170.0;
        missingName.weight = 60.0;
        missingName.age = 25;
        error.clear();
        const bool missingInserted = model.insertPatient(missingName, &error);
        check(!missingInserted && error.contains(QStringLiteral("姓名")),
              QStringLiteral("缺失姓名被拒绝"), error);

        QSortFilterProxyModel proxy;
        proxy.setSourceModel(&model);
        proxy.setFilterKeyColumn(-1);
        proxy.setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy.setFilterFixedString(QStringLiteral("NO_SUCH_PATIENT_123"));
        check(proxy.rowCount() == 0, QStringLiteral("查询无结果时返回空集"),
              QStringLiteral("proxy.rowCount() = %1").arg(proxy.rowCount()));

        Patient edited = model.patientAt(0);
        const QString originalName = edited.name;
        edited.name = originalName + QStringLiteral("·测试");
        error.clear();
        const bool updated = model.updatePatient(edited, &error);
        model.select();
        const QString refreshedName = model.patientAt(0).name;
        check(updated && refreshedName == edited.name, QStringLiteral("修改后模型刷新"),
              QStringLiteral("更新为 %1，重新查询得到 %2").arg(edited.name, refreshedName));
        edited.name = originalName;
        model.updatePatient(edited, &error);
        model.select();

        Patient temp;
        temp.name = QStringLiteral("临时验证患者");
        temp.idCard = QStringLiteral("440101200001010099");
        temp.sex = QStringLiteral("女");
        temp.dob = QStringLiteral("2000-01-01");
        temp.mobile = QStringLiteral("13900000000");
        temp.height = 165.0;
        temp.weight = 55.0;
        temp.age = 25;
        const int before = model.rowCount();
        error.clear();
        const bool inserted = model.insertPatient(temp, &error);
        const int after = model.rowCount();
        check(inserted && after == before + 1, QStringLiteral("新增记录写入数据库"),
              QStringLiteral("%1 -> %2").arg(before).arg(after));

        QSqlQuery idQuery(DatabaseManager::instance().database());
        idQuery.prepare(QStringLiteral("SELECT id FROM patient WHERE id_card = ?"));
        idQuery.addBindValue(temp.idCard);
        int tempId = -1;
        if (idQuery.exec() && idQuery.next()) {
            tempId = idQuery.value(0).toInt();
        }
        error.clear();
        const bool removed = model.removePatient(tempId, &error);
        check(removed && model.rowCount() == before, QStringLiteral("删除记录后行数恢复"),
              QStringLiteral("删除 id=%1，当前行数 %2").arg(tempId).arg(model.rowCount()));

        QString probeError;
        const bool probeOk = DatabaseManager::probe(
            QStringLiteral("Z:/definitely/not/here/hospital.db"), &probeError);
        check(!probeOk && !probeError.isEmpty(), QStringLiteral("无效数据库路径被拒绝"), probeError);

        DatabaseManager::instance().logEvent(QStringLiteral("system"),
                                             QStringLiteral("自测完成：失败项 %1").arg(failures));
    }

    const QString outputPath = QDir(QCoreApplication::applicationDirPath())
                                   .filePath(QStringLiteral("selftest.txt"));
    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        for (const QString &line : results) {
            stream << line << '\n';
        }
        stream << QStringLiteral("RESULT: %1, FAILURES: %2\n")
                      .arg(failures == 0 ? QStringLiteral("PASS") : QStringLiteral("FAIL"))
                      .arg(failures);
    }

    for (const QString &line : results) {
        const QByteArray bytes = line.toUtf8();
        printf("%s\n", bytes.constData());
    }
    printf("RESULT: %s, FAILURES: %d\n", failures == 0 ? "PASS" : "FAIL", failures);

    DatabaseManager::instance().close();
    removeDatabaseFiles(databasePath);
    return failures == 0 ? 0 : 2;
}
