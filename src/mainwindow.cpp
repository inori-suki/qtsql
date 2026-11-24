#include "mainwindow.h"

#include "database.h"
#include "loginpage.h"
#include "workspace.h"

#include <QLineEdit>
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("医院诊疗信息管理系统 · Qt SQL / Model-View 实验"));
    resize(1280, 820);
    setMinimumSize(1120, 720);

    m_stack = new QStackedWidget;
    m_loginPage = new LoginPage;
    m_workspace = new WorkspacePage;
    m_stack->addWidget(m_loginPage);
    m_stack->addWidget(m_workspace);
    setCentralWidget(m_stack);

    connect(m_loginPage, &LoginPage::loginRequested, this, &MainWindow::handleLogin);
    connect(m_workspace, &WorkspacePage::logoutRequested, this, &MainWindow::logout);

    m_stack->setCurrentIndex(0);
}

void MainWindow::handleLogin(const QString &username, const QString &password)
{
    tryLogin(username, password);
}

bool MainWindow::tryLogin(const QString &username, const QString &password)
{
    m_loginPage->clearError();
    if (username.trimmed().isEmpty()) {
        m_loginPage->setError(QStringLiteral("请输入用户名"));
        return false;
    }
    if (password.isEmpty()) {
        m_loginPage->setError(QStringLiteral("请输入密码"));
        return false;
    }

    UserInfo user;
    QString error;
    if (!DatabaseManager::instance().authenticate(username, password, &user, &error)) {
        DatabaseManager::instance().logEvent(username, QStringLiteral("登录失败：凭据错误"));
        m_loginPage->setError(error.isEmpty() ? QStringLiteral("用户名或密码错误") : error);
        return false;
    }

    DatabaseManager::instance().logEvent(user.username, QStringLiteral("登录成功"));
    m_workspace->setUser(user);
    m_workspace->refreshAll();
    m_stack->setCurrentIndex(1);
    return true;
}

void MainWindow::logout()
{
    m_stack->setCurrentIndex(0);
    m_loginPage->setCredentials(QString(), QString());
    m_loginPage->clearError();
    m_loginPage->passwordEdit()->clear();
}
