#pragma once

#include "database.h"
#include "patient.h"

#include <QWidget>

class PatientModel;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QSqlQueryModel;
class QStackedWidget;
class QTableView;

class PatientPage : public QWidget {
    Q_OBJECT

public:
    explicit PatientPage(QWidget *parent = nullptr);

    void setUser(const UserInfo &user);
    void refresh();
    PatientModel *model() const { return m_model; }

    void setSearchText(const QString &text);
    void clearSearch();
    void applySearch();
    void selectRow(int proxyRow);
    Patient selectedPatient() const;

private slots:
    void addPatient();
    void editPatient();
    void deletePatient();
    void onDoubleClicked(const QModelIndex &index);

private:
    void updateStatus();
    void setStatus(const QString &text, const QString &color = QStringLiteral("#7A8A99"));
    void logEvent(const QString &event);

    PatientModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxy = nullptr;
    QTableView *m_table = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_searchField = nullptr;
    QLabel *m_countLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    UserInfo m_user;
};

class SimpleTablePage : public QWidget {
    Q_OBJECT

public:
    SimpleTablePage(const QString &title, const QString &subtitle,
                    const QString &query, QWidget *parent = nullptr);
    void refresh();

private:
    QString m_query;
    QSqlQueryModel *m_model = nullptr;
    QTableView *m_table = nullptr;
    QLabel *m_status = nullptr;
};

class WorkspacePage : public QWidget {
    Q_OBJECT

public:
    explicit WorkspacePage(QWidget *parent = nullptr);

    void setUser(const UserInfo &user);
    void refreshAll();
    void showPage(int index);
    PatientPage *patientPage() const { return m_patientPage; }
    int currentPage() const;

signals:
    void logoutRequested();

private:
    PatientPage *m_patientPage = nullptr;
    SimpleTablePage *m_doctorPage = nullptr;
    SimpleTablePage *m_departmentPage = nullptr;
    SimpleTablePage *m_logPage = nullptr;
    QStackedWidget *m_stack = nullptr;
    QList<QPushButton *> m_navButtons;
    QLabel *m_userLabel = nullptr;
};
