#pragma once

#include "patient.h"

#include <QDialog>

class PatientModel;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;

class PatientDialog : public QDialog {
    Q_OBJECT

public:
    explicit PatientDialog(QWidget *parent, PatientModel *model, const Patient *patient = nullptr);

    Patient patient() const;
    void attemptSave();
    void showError(const QString &message);

    void setName(const QString &name);
    void setIdCard(const QString &idCard);
    void setMobile(const QString &mobile);

private slots:
    void updateAge();

private:
    void setHeaderText(const QString &title, const QString &subtitle);

    PatientModel *m_model = nullptr;
    int m_patientId = -1;

    QLineEdit *m_name = nullptr;
    QLineEdit *m_idCard = nullptr;
    QComboBox *m_sex = nullptr;
    QDateEdit *m_dob = nullptr;
    QDoubleSpinBox *m_height = nullptr;
    QDoubleSpinBox *m_weight = nullptr;
    QLineEdit *m_mobile = nullptr;
    QLineEdit *m_age = nullptr;

    QLabel *m_title = nullptr;
    QLabel *m_subtitle = nullptr;
    QLabel *m_error = nullptr;
};

class ConfirmDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfirmDialog(QWidget *parent, const QString &patientName);
    bool confirmed() const { return m_confirmed; }

private:
    bool m_confirmed = false;
};

class ErrorDialog : public QDialog {
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent, const QString &title,
                         const QString &message, const QString &detail);
};
