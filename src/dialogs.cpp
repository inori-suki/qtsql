#include "dialogs.h"

#include "patientmodel.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

QWidget *fieldBox(const QString &labelText, QWidget *editor)
{
    auto *container = new QWidget;
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    auto *label = new QLabel(labelText);
    label->setObjectName(QStringLiteral("FieldLabel"));
    layout->addWidget(label);
    layout->addWidget(editor);
    return container;
}

} // namespace

PatientDialog::PatientDialog(QWidget *parent, PatientModel *model, const Patient *patient)
    : QDialog(parent)
    , m_model(model)
{
    setObjectName(QStringLiteral("PatientDialog"));
    setWindowTitle(patient ? QStringLiteral("编辑患者信息") : QStringLiteral("新增患者"));
    setModal(true);
    setMinimumWidth(760);
    resize(780, 600);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 22);
    root->setSpacing(16);

    auto *header = new QHBoxLayout;
    auto *texts = new QVBoxLayout;
    texts->setSpacing(3);
    m_title = new QLabel;
    m_title->setObjectName(QStringLiteral("DialogTitle"));
    m_subtitle = new QLabel;
    m_subtitle->setObjectName(QStringLiteral("DialogSubtitle"));
    texts->addWidget(m_title);
    texts->addWidget(m_subtitle);
    header->addLayout(texts);
    header->addStretch();
    auto *idChip = new QLabel(patient ? QStringLiteral("EDIT") : QStringLiteral("NEW"));
    idChip->setObjectName(QStringLiteral("CountLabel"));
    header->addWidget(idChip, 0, Qt::AlignTop);
    root->addLayout(header);

    m_error = new QLabel;
    m_error->setObjectName(QStringLiteral("ErrorBanner"));
    m_error->setWordWrap(true);
    m_error->hide();
    root->addWidget(m_error);

    m_name = new QLineEdit;
    m_name->setPlaceholderText(QStringLiteral("请输入患者姓名"));
    m_name->setMaxLength(20);

    m_idCard = new QLineEdit;
    m_idCard->setPlaceholderText(QStringLiteral("18位身份证号"));
    m_idCard->setMaxLength(18);

    m_sex = new QComboBox;
    m_sex->addItems({QStringLiteral("男"), QStringLiteral("女")});

    m_dob = new QDateEdit;
    m_dob->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_dob->setCalendarPopup(true);
    m_dob->setDateRange(QDate(1900, 1, 1), QDate::currentDate());
    m_dob->setDate(QDate(1995, 1, 1));

    m_height = new QDoubleSpinBox;
    m_height->setRange(30.0, 250.0);
    m_height->setDecimals(1);
    m_height->setSingleStep(0.5);
    m_height->setSuffix(QStringLiteral(" cm"));
    m_height->setValue(170.0);

    m_weight = new QDoubleSpinBox;
    m_weight->setRange(2.0, 300.0);
    m_weight->setDecimals(1);
    m_weight->setSingleStep(0.5);
    m_weight->setSuffix(QStringLiteral(" kg"));
    m_weight->setValue(60.0);

    m_mobile = new QLineEdit;
    m_mobile->setPlaceholderText(QStringLiteral("11位手机号"));
    m_mobile->setMaxLength(11);

    m_age = new QLineEdit;
    m_age->setReadOnly(true);
    m_age->setPlaceholderText(QStringLiteral("根据出生日期自动计算"));

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(22);
    grid->setVerticalSpacing(13);
    grid->addWidget(fieldBox(QStringLiteral("姓名 *"), m_name), 0, 0);
    grid->addWidget(fieldBox(QStringLiteral("身份证号 *"), m_idCard), 0, 1);
    grid->addWidget(fieldBox(QStringLiteral("性别 *"), m_sex), 1, 0);
    grid->addWidget(fieldBox(QStringLiteral("出生日期 *"), m_dob), 1, 1);
    grid->addWidget(fieldBox(QStringLiteral("身高"), m_height), 2, 0);
    grid->addWidget(fieldBox(QStringLiteral("体重"), m_weight), 2, 1);
    grid->addWidget(fieldBox(QStringLiteral("手机号 *"), m_mobile), 3, 0);
    grid->addWidget(fieldBox(QStringLiteral("年龄（自动计算）"), m_age), 3, 1);
    root->addLayout(grid);

    auto *tip = new QLabel(QStringLiteral("带 * 的字段为必填项；身份证号需唯一，保存前会进行格式与重复校验。"));
    tip->setObjectName(QStringLiteral("HintLabel"));
    tip->setWordWrap(true);
    root->addWidget(tip);
    root->addStretch();

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    auto *cancel = new QPushButton(QStringLiteral("取消"));
    cancel->setObjectName(QStringLiteral("GhostButton"));
    cancel->setMinimumWidth(96);
    auto *save = new QPushButton(QStringLiteral("保存"));
    save->setObjectName(QStringLiteral("PrimaryButton"));
    save->setMinimumWidth(120);
    buttons->addWidget(cancel);
    buttons->addWidget(save);
    root->addLayout(buttons);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(save, &QPushButton::clicked, this, &PatientDialog::attemptSave);
    connect(m_dob, &QDateEdit::dateChanged, this, &PatientDialog::updateAge);

    if (patient) {
        setHeaderText(QStringLiteral("编辑患者信息"), QStringLiteral("修改后的数据会立即写回 SQLite 数据库"));
        m_patientId = patient->id;
        m_name->setText(patient->name);
        m_idCard->setText(patient->idCard);
        m_sex->setCurrentText(patient->sex);
        const QDate dob = QDate::fromString(patient->dob, QStringLiteral("yyyy-MM-dd"));
        if (dob.isValid()) {
            m_dob->setDate(dob);
        }
        m_height->setValue(patient->height > 0 ? patient->height : 170.0);
        m_weight->setValue(patient->weight > 0 ? patient->weight : 60.0);
        m_mobile->setText(patient->mobile);
    } else {
        setHeaderText(QStringLiteral("新增患者"), QStringLiteral("为患者建立档案，保存后立即出现在列表中"));
    }
    updateAge();
}

void PatientDialog::setHeaderText(const QString &title, const QString &subtitle)
{
    m_title->setText(title);
    m_subtitle->setText(subtitle);
}

void PatientDialog::setName(const QString &name)
{
    m_name->setText(name);
}

void PatientDialog::setIdCard(const QString &idCard)
{
    m_idCard->setText(idCard);
}

void PatientDialog::setMobile(const QString &mobile)
{
    m_mobile->setText(mobile);
}

void PatientDialog::updateAge()
{
    const QDate dob = m_dob->date();
    const QDate today = QDate::currentDate();
    int age = today.year() - dob.year();
    if (today < dob.addYears(age)) {
        --age;
    }
    m_age->setText(QStringLiteral("%1 岁").arg(qMax(0, age)));
}

Patient PatientDialog::patient() const
{
    Patient p;
    p.id = m_patientId;
    p.name = m_name->text().trimmed();
    p.idCard = m_idCard->text().trimmed().toUpper();
    p.sex = m_sex->currentText();
    p.dob = m_dob->date().toString(QStringLiteral("yyyy-MM-dd"));
    p.height = m_height->value();
    p.weight = m_weight->value();
    p.mobile = m_mobile->text().trimmed();
    const QDate today = QDate::currentDate();
    int age = today.year() - m_dob->date().year();
    if (today < m_dob->date().addYears(age)) {
        --age;
    }
    p.age = qMax(0, age);
    return p;
}

void PatientDialog::showError(const QString &message)
{
    m_error->setText(message);
    m_error->setVisible(!message.isEmpty());
}

void PatientDialog::attemptSave()
{
    showError(QString());
    Patient p = patient();
    const QString validation = PatientModel::validatePatient(p);
    if (!validation.isEmpty()) {
        showError(validation);
        return;
    }

    QString duplicateError;
    const bool duplicated = m_model->idCardExists(p.idCard, m_patientId, &duplicateError);
    if (duplicated || !duplicateError.isEmpty()) {
        showError(duplicateError.isEmpty() ? QStringLiteral("身份证号已存在") : duplicateError);
        return;
    }
    accept();
}

ConfirmDialog::ConfirmDialog(QWidget *parent, const QString &patientName)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("ConfirmDialog"));
    setWindowTitle(QStringLiteral("删除确认"));
    setModal(true);
    setFixedSize(560, 250);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 26, 28, 22);
    root->setSpacing(14);

    auto *top = new QHBoxLayout;
    auto *icon = new QLabel(QStringLiteral("!"));
    icon->setObjectName(QStringLiteral("WarnIcon"));
    icon->setFixedSize(52, 52);
    icon->setAlignment(Qt::AlignCenter);
    top->addWidget(icon, 0, Qt::AlignTop);

    auto *texts = new QVBoxLayout;
    texts->setSpacing(5);
    auto *title = new QLabel(QStringLiteral("确认删除该患者档案？"));
    title->setObjectName(QStringLiteral("DialogTitle"));
    auto *message = new QLabel(
        QStringLiteral("即将删除患者「%1」的档案。删除后该记录将从数据库和列表中移除，且无法恢复。")
            .arg(patientName));
    message->setWordWrap(true);
    message->setObjectName(QStringLiteral("PageSubtitle"));
    texts->addWidget(title);
    texts->addWidget(message);
    top->addLayout(texts, 1);
    root->addLayout(top);

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    auto *cancel = new QPushButton(QStringLiteral("取消"));
    cancel->setObjectName(QStringLiteral("GhostButton"));
    cancel->setMinimumWidth(96);
    auto *confirm = new QPushButton(QStringLiteral("确认删除"));
    confirm->setObjectName(QStringLiteral("DangerButton"));
    confirm->setMinimumWidth(120);
    buttons->addWidget(cancel);
    buttons->addWidget(confirm);
    root->addLayout(buttons);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, this, [this]() {
        m_confirmed = true;
        accept();
    });
}

ErrorDialog::ErrorDialog(QWidget *parent, const QString &title,
                         const QString &message, const QString &detail)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("ErrorDialog"));
    setWindowTitle(title);
    setModal(true);
    setFixedWidth(560);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 26, 28, 22);
    root->setSpacing(14);

    auto *top = new QHBoxLayout;
    auto *icon = new QLabel(QStringLiteral("!"));
    icon->setObjectName(QStringLiteral("DangerIcon"));
    icon->setFixedSize(52, 52);
    icon->setAlignment(Qt::AlignCenter);
    top->addWidget(icon, 0, Qt::AlignTop);

    auto *texts = new QVBoxLayout;
    texts->setSpacing(5);
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName(QStringLiteral("DialogTitle"));
    auto *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setObjectName(QStringLiteral("PageSubtitle"));
    texts->addWidget(titleLabel);
    texts->addWidget(messageLabel);
    top->addLayout(texts, 1);
    root->addLayout(top);

    auto *detailFrame = new QFrame;
    detailFrame->setObjectName(QStringLiteral("Card"));
    auto *detailLayout = new QVBoxLayout(detailFrame);
    detailLayout->setContentsMargins(12, 10, 12, 10);
    auto *detailLabel = new QLabel(detail);
    detailLabel->setWordWrap(true);
    detailLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailLabel->setObjectName(QStringLiteral("HintLabel"));
    detailLayout->addWidget(detailLabel);
    root->addWidget(detailFrame);

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    auto *close = new QPushButton(QStringLiteral("关闭"));
    close->setObjectName(QStringLiteral("PrimaryButton"));
    close->setMinimumWidth(110);
    buttons->addWidget(close);
    root->addLayout(buttons);

    connect(close, &QPushButton::clicked, this, &QDialog::accept);
}
