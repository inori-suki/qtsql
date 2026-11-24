#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class LoginPage : public QWidget {
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);

    void setCredentials(const QString &username, const QString &password);
    void setError(const QString &message);
    void clearError();
    void submit();
    QString username() const;
    QLineEdit *passwordEdit() const { return m_password; }

signals:
    void loginRequested(const QString &username, const QString &password);

private:
    QLineEdit *m_username = nullptr;
    QLineEdit *m_password = nullptr;
    QLabel *m_error = nullptr;
    QPushButton *m_loginButton = nullptr;
};
