#pragma once

#include <QMainWindow>

class LoginPage;
class QStackedWidget;
class WorkspacePage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    LoginPage *loginPage() const { return m_loginPage; }
    WorkspacePage *workspace() const { return m_workspace; }
    bool tryLogin(const QString &username, const QString &password);

private slots:
    void handleLogin(const QString &username, const QString &password);
    void logout();

private:
    QStackedWidget *m_stack = nullptr;
    LoginPage *m_loginPage = nullptr;
    WorkspacePage *m_workspace = nullptr;
};
