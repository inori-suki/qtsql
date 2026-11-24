#include "loginpage.h"

#include "config.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("LoginPage"));
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *brand = new QFrame;
    brand->setObjectName(QStringLiteral("BrandPanel"));
    brand->setFixedWidth(480);
    auto *brandLayout = new QVBoxLayout(brand);
    brandLayout->setContentsMargins(52, 54, 52, 48);
    brandLayout->setSpacing(14);

    auto *kicker = new QLabel(QStringLiteral("DONGGUAN UNIVERSITY OF TECHNOLOGY"));
    kicker->setObjectName(QStringLiteral("BrandKicker"));
    brandLayout->addWidget(kicker);

    auto *title = new QLabel(QStringLiteral("医院诊疗\n信息管理系统"));
    title->setObjectName(QStringLiteral("BrandTitle"));
    title->setWordWrap(true);
    brandLayout->addWidget(title);

    auto *accent = new QFrame;
    accent->setFixedSize(64, 3);
    accent->setStyleSheet(QStringLiteral("background:#2A9D8F;border:none;"));
    brandLayout->addWidget(accent);

    auto *subtitle = new QLabel(QStringLiteral("Hospital Information Management System"));
    subtitle->setObjectName(QStringLiteral("BrandSubtitle"));
    brandLayout->addWidget(subtitle);

    brandLayout->addSpacing(24);
    const QStringList bullets = {
        QStringLiteral("·  患者档案统一管理，快速检索"),
        QStringLiteral("·  登录校验与操作留痕"),
        QStringLiteral("·  Qt 6 + SQLite + Model/View"),
    };
    for (const QString &text : bullets) {
        auto *label = new QLabel(text);
        label->setObjectName(QStringLiteral("BrandBullet"));
        brandLayout->addWidget(label);
    }
    brandLayout->addStretch();

    auto *footer = new QLabel(QStringLiteral("课程实验 · 2026 秋季学期"));
    footer->setObjectName(QStringLiteral("BrandFooter"));
    brandLayout->addWidget(footer);

    root->addWidget(brand);

    auto *formHost = new QWidget;
    auto *formHostLayout = new QVBoxLayout(formHost);
    formHostLayout->setContentsMargins(40, 32, 40, 28);
    formHostLayout->setSpacing(12);
    formHostLayout->addStretch();

    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("LoginCard"));
    card->setFixedWidth(440);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(38, 34, 38, 30);
    cardLayout->setSpacing(10);

    auto *titleLabel = new QLabel(QStringLiteral("欢迎登录"));
    titleLabel->setObjectName(QStringLiteral("LoginTitle"));
    cardLayout->addWidget(titleLabel);

    auto *subtitleLabel = new QLabel(QStringLiteral("请输入账号和密码，进入患者管理工作台"));
    subtitleLabel->setObjectName(QStringLiteral("LoginSubtitle"));
    cardLayout->addWidget(subtitleLabel);
    cardLayout->addSpacing(14);

    auto *userLabel = new QLabel(QStringLiteral("用户名"));
    userLabel->setObjectName(QStringLiteral("FieldLabel"));
    cardLayout->addWidget(userLabel);
    m_username = new QLineEdit;
    m_username->setObjectName(QStringLiteral("Input"));
    m_username->setPlaceholderText(QStringLiteral("请输入用户名"));
    m_username->setText(AppConfig::kDemoUser);
    cardLayout->addWidget(m_username);

    auto *passwordLabel = new QLabel(QStringLiteral("密码"));
    passwordLabel->setObjectName(QStringLiteral("FieldLabel"));
    cardLayout->addWidget(passwordLabel);
    m_password = new QLineEdit;
    m_password->setObjectName(QStringLiteral("Input"));
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setPlaceholderText(QStringLiteral("请输入密码"));
    m_password->setText(AppConfig::kDemoPassword);
    cardLayout->addWidget(m_password);

    m_error = new QLabel;
    m_error->setObjectName(QStringLiteral("ErrorLabel"));
    m_error->setWordWrap(true);
    m_error->hide();
    cardLayout->addWidget(m_error);

    cardLayout->addSpacing(8);
    m_loginButton = new QPushButton(QStringLiteral("登 录"));
    m_loginButton->setObjectName(QStringLiteral("PrimaryButton"));
    m_loginButton->setMinimumHeight(44);
    m_loginButton->setDefault(true);
    cardLayout->addWidget(m_loginButton);

    auto *hint = new QLabel(QStringLiteral("演示账号：admin / 123456　　doctor / 123456"));
    hint->setObjectName(QStringLiteral("HintLabel"));
    hint->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(hint);

    cardLayout->addStretch();
    formHostLayout->addWidget(card, 0, Qt::AlignHCenter);

    auto *developer = new QLabel(
        QStringLiteral("开发者：%1　学号：%2").arg(AppConfig::kStudentName, AppConfig::kStudentId));
    developer->setObjectName(QStringLiteral("DeveloperLabel"));
    developer->setAlignment(Qt::AlignCenter);
    formHostLayout->addWidget(developer);
    formHostLayout->addStretch();

    root->addWidget(formHost, 1);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginPage::submit);
    connect(m_password, &QLineEdit::returnPressed, this, &LoginPage::submit);
    connect(m_username, &QLineEdit::returnPressed, this, &LoginPage::submit);
    setTabOrder(m_username, m_password);
    setTabOrder(m_password, m_loginButton);
}

void LoginPage::setCredentials(const QString &username, const QString &password)
{
    m_username->setText(username);
    m_password->setText(password);
}

void LoginPage::setError(const QString &message)
{
    m_error->setText(message);
    m_error->setVisible(!message.isEmpty());
}

void LoginPage::clearError()
{
    m_error->clear();
    m_error->hide();
}

void LoginPage::submit()
{
    clearError();
    if (m_username->text().trimmed().isEmpty()) {
        setError(QStringLiteral("请输入用户名"));
        m_username->setFocus();
        return;
    }
    if (m_password->text().isEmpty()) {
        setError(QStringLiteral("请输入密码"));
        m_password->setFocus();
        return;
    }
    emit loginRequested(m_username->text().trimmed(), m_password->text());
}

QString LoginPage::username() const
{
    return m_username->text().trimmed();
}
