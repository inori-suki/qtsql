#include "patientmodel.h"

#include "database.h"

#include <QDate>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

PatientModel::PatientModel(QObject *parent)
    : QSqlTableModel(parent, DatabaseManager::instance().database())
{
    setTable(QStringLiteral("patient"));
    setEditStrategy(QSqlTableModel::OnFieldChange);
    setSort(0, Qt::AscendingOrder);
    select();
}

QVariant PatientModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        const QString field = record().fieldName(section);
        if (field == QStringLiteral("id")) return QStringLiteral("编号");
        if (field == QStringLiteral("name")) return QStringLiteral("姓名");
        if (field == QStringLiteral("id_card")) return QStringLiteral("身份证号");
        if (field == QStringLiteral("sex")) return QStringLiteral("性别");
        if (field == QStringLiteral("age")) return QStringLiteral("年龄");
        if (field == QStringLiteral("dob")) return QStringLiteral("出生日期");
        if (field == QStringLiteral("height")) return QStringLiteral("身高(cm)");
        if (field == QStringLiteral("weight")) return QStringLiteral("体重(kg)");
        if (field == QStringLiteral("mobile")) return QStringLiteral("手机号");
        if (field == QStringLiteral("created_at")) return QStringLiteral("建档时间");
    }
    return QSqlTableModel::headerData(section, orientation, role);
}

int PatientModel::columnIndex(const QString &fieldName) const
{
    return fieldIndex(fieldName);
}

Patient PatientModel::patientAt(int row) const
{
    Patient patient;
    if (row < 0 || row >= rowCount()) {
        return patient;
    }
    const QSqlRecord rec = record(row);
    patient.id = rec.value(QStringLiteral("id")).toInt();
    patient.name = rec.value(QStringLiteral("name")).toString();
    patient.idCard = rec.value(QStringLiteral("id_card")).toString();
    patient.sex = rec.value(QStringLiteral("sex")).toString();
    patient.dob = rec.value(QStringLiteral("dob")).toString();
    patient.height = rec.value(QStringLiteral("height")).toDouble();
    patient.weight = rec.value(QStringLiteral("weight")).toDouble();
    patient.mobile = rec.value(QStringLiteral("mobile")).toString();
    patient.age = rec.value(QStringLiteral("age")).toInt();
    return patient;
}

QString PatientModel::validatePatient(const Patient &patient)
{
    if (patient.name.trimmed().isEmpty()) {
        return QStringLiteral("姓名不能为空");
    }
    if (patient.name.trimmed().size() > 20) {
        return QStringLiteral("姓名长度不能超过20个字符");
    }
    static const QRegularExpression idPattern(QStringLiteral("^\\d{17}[\\dXx]$"));
    if (!idPattern.match(patient.idCard.trimmed()).hasMatch()) {
        return QStringLiteral("身份证号应为18位，最后一位可为X");
    }
    if (patient.sex != QStringLiteral("男") && patient.sex != QStringLiteral("女")) {
        return QStringLiteral("请选择性别");
    }
    const QDate dob = QDate::fromString(patient.dob, QStringLiteral("yyyy-MM-dd"));
    if (!dob.isValid() || dob > QDate::currentDate() || dob.year() < 1900) {
        return QStringLiteral("出生日期不正确");
    }
    static const QRegularExpression mobilePattern(QStringLiteral("^1[3-9]\\d{9}$"));
    if (!mobilePattern.match(patient.mobile.trimmed()).hasMatch()) {
        return QStringLiteral("手机号应为11位，且以1开头");
    }
    if (patient.height < 30.0 || patient.height > 250.0) {
        return QStringLiteral("身高应在30～250cm之间");
    }
    if (patient.weight < 2.0 || patient.weight > 300.0) {
        return QStringLiteral("体重应在2～300kg之间");
    }
    return {};
}

bool PatientModel::idCardExists(const QString &idCard, int excludeId, QString *error) const
{
    return checkDuplicate(idCard, excludeId, error);
}

bool PatientModel::checkDuplicate(const QString &idCard, int excludeId, QString *error) const
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM patient WHERE id_card = ? AND id <> ?"));
    query.addBindValue(idCard.trimmed());
    query.addBindValue(excludeId);
    if (!query.exec() || !query.next()) {
        if (error) {
            *error = QStringLiteral("身份证号查重失败：%1").arg(query.lastError().text());
        }
        return false;
    }
    if (query.value(0).toInt() > 0) {
        if (error) {
            *error = QStringLiteral("身份证号已存在，请勿重复建档");
        }
        return true;
    }
    return false;
}

bool PatientModel::insertPatient(const Patient &patient, QString *error)
{
    const QString validation = validatePatient(patient);
    if (!validation.isEmpty()) {
        if (error) *error = validation;
        return false;
    }
    if (checkDuplicate(patient.idCard, -1, error)) {
        return false;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral(
        "INSERT INTO patient(id_card, name, sex, dob, height, weight, mobile, age, created_at)"
        " VALUES(?, ?, ?, ?, ?, ?, ?, ?, datetime('now', 'localtime'))"));
    query.addBindValue(patient.idCard.trimmed().toUpper());
    query.addBindValue(patient.name.trimmed());
    query.addBindValue(patient.sex);
    query.addBindValue(patient.dob);
    query.addBindValue(patient.height);
    query.addBindValue(patient.weight);
    query.addBindValue(patient.mobile.trimmed());
    query.addBindValue(patient.age);
    if (!query.exec()) {
        if (error) *error = QStringLiteral("新增患者失败：%1").arg(query.lastError().text());
        return false;
    }
    select();
    return true;
}

bool PatientModel::updatePatient(const Patient &patient, QString *error)
{
    if (patient.id <= 0) {
        if (error) *error = QStringLiteral("缺少患者编号，无法修改");
        return false;
    }
    const QString validation = validatePatient(patient);
    if (!validation.isEmpty()) {
        if (error) *error = validation;
        return false;
    }
    if (checkDuplicate(patient.idCard, patient.id, error)) {
        return false;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral(
        "UPDATE patient SET id_card = ?, name = ?, sex = ?, dob = ?, height = ?,"
        " weight = ?, mobile = ?, age = ? WHERE id = ?"));
    query.addBindValue(patient.idCard.trimmed().toUpper());
    query.addBindValue(patient.name.trimmed());
    query.addBindValue(patient.sex);
    query.addBindValue(patient.dob);
    query.addBindValue(patient.height);
    query.addBindValue(patient.weight);
    query.addBindValue(patient.mobile.trimmed());
    query.addBindValue(patient.age);
    query.addBindValue(patient.id);
    if (!query.exec()) {
        if (error) *error = QStringLiteral("修改患者失败：%1").arg(query.lastError().text());
        return false;
    }
    select();
    return true;
}

bool PatientModel::removePatient(int id, QString *error)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("DELETE FROM patient WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec()) {
        if (error) *error = QStringLiteral("删除患者失败：%1").arg(query.lastError().text());
        return false;
    }
    select();
    return true;
}
