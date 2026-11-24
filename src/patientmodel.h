#pragma once

#include "patient.h"

#include <QSqlTableModel>

class PatientModel : public QSqlTableModel {
    Q_OBJECT

public:
    explicit PatientModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int columnIndex(const QString &fieldName) const;
    Patient patientAt(int row) const;

    bool insertPatient(const Patient &patient, QString *error);
    bool updatePatient(const Patient &patient, QString *error);
    bool removePatient(int id, QString *error);
    bool idCardExists(const QString &idCard, int excludeId, QString *error) const;

    static QString validatePatient(const Patient &patient);

private:
    bool checkDuplicate(const QString &idCard, int excludeId, QString *error) const;
};
