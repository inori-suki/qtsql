#include "workspace.h"

#include "config.h"
#include "dialogs.h"
#include "patientmodel.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDate>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>

namespace {

class SexDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                          const QModelIndex &) const override
    {
        auto *editor = new QComboBox(parent);
        editor->addItems({QStringLiteral("男"), QStringLiteral("女")});
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        auto *combo = qobject_cast<QComboBox *>(editor);
        if (combo) {
            combo->setCurrentText(index.data(Qt::EditRole).toString());
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        auto *combo = qobject_cast<QComboBox *>(editor);
        if (combo) {
            model->setData(index, combo->currentText(), Qt::EditRole);
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &) const override
    {
        editor->setGeometry(option.rect);
    }
};

QFrame *makeCard()
{
    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("Card"));
    return card;
}

} // namespace

PatientPage::PatientPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("PatientPage"));
    m_model = new PatientModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 22, 24, 18);
    root->setSpacing(14);

    auto *titleRow = new QHBoxLayout;
    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(4);
    auto *title = new QLabel(QStringLiteral("患者档案管理"));
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("查询、建档、修改与删除患者信息，表格数据与 SQLite 数据库实时同步"));
    subtitle->setObjectName(QStringLiteral("PageSubtitle"));
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    titleRow->addLayout(titleBox);
    titleRow->addStretch();
    m_countLabel = new QLabel;
    m_countLabel->setObjectName(QStringLiteral("CountLabel"));
    titleRow->addWidget(m_countLabel, 0, Qt::AlignBottom);
    root->addLayout(titleRow);

    auto *toolbar = makeCard();
    auto *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(14, 12, 14, 12);
    toolbarLayout->setSpacing(9);

    auto *addButton = new QPushButton(QStringLiteral("＋ 新增患者"));
    addButton->setObjectName(QStringLiteral("PrimaryButton"));
    addButton->setProperty("captureName", "addButton");
    auto *editButton = new QPushButton(QStringLiteral("编辑"));
    editButton->setObjectName(QStringLiteral("SecondaryButton"));
    auto *deleteButton = new QPushButton(QStringLiteral("删除"));
    deleteButton->setObjectName(QStringLiteral("DangerButton"));
    auto *refreshButton = new QPushButton(QStringLiteral("刷新"));
    refreshButton->setObjectName(QStringLiteral("GhostButton"));
    toolbarLayout->addWidget(addButton);
    toolbarLayout->addWidget(editButton);
    toolbarLayout->addWidget(deleteButton);
    toolbarLayout->addWidget(refreshButton);
    toolbarLayout->addSpacing(12);

    m_searchField = new QComboBox;
    m_searchField->addItems({QStringLiteral("全部字段"), QStringLiteral("姓名"),
                             QStringLiteral("身份证号"), QStringLiteral("手机号")});
    m_searchField->setMinimumWidth(108);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(QStringLiteral("输入关键词查询患者…"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(220);
    auto *searchButton = new QPushButton(QStringLiteral("查询"));
    searchButton->setObjectName(QStringLiteral("PrimaryButton"));
    auto *resetButton = new QPushButton(QStringLiteral("重置"));
    resetButton->setObjectName(QStringLiteral("GhostButton"));
    toolbarLayout->addWidget(m_searchField);
    toolbarLayout->addWidget(m_searchEdit, 1);
    toolbarLayout->addWidget(searchButton);
    toolbarLayout->addWidget(resetButton);
    root->addWidget(toolbar);

    auto *tableCard = makeCard();
    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(0);

    m_table = new QTableView;
    m_table->setObjectName(QStringLiteral("patientTable"));
    m_table->setModel(m_proxy);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked
                             | QAbstractItemView::EditKeyPressed
                             | QAbstractItemView::SelectedClicked);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(0, Qt::AscendingOrder);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(39);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setMinimumSectionSize(56);
    m_table->setItemDelegateForColumn(m_model->columnIndex(QStringLiteral("sex")), new SexDelegate(m_table));

    const int idCol = m_model->columnIndex(QStringLiteral("id"));
    const int nameCol = m_model->columnIndex(QStringLiteral("name"));
    const int idCardCol = m_model->columnIndex(QStringLiteral("id_card"));
    const int sexCol = m_model->columnIndex(QStringLiteral("sex"));
    const int ageCol = m_model->columnIndex(QStringLiteral("age"));
    const int dobCol = m_model->columnIndex(QStringLiteral("dob"));
    const int heightCol = m_model->columnIndex(QStringLiteral("height"));
    const int weightCol = m_model->columnIndex(QStringLiteral("weight"));
    const int mobileCol = m_model->columnIndex(QStringLiteral("mobile"));
    m_table->setColumnWidth(idCol, 62);
    m_table->setColumnWidth(nameCol, 92);
    m_table->setColumnWidth(idCardCol, 172);
    m_table->setColumnWidth(sexCol, 62);
    m_table->setColumnWidth(ageCol, 58);
    m_table->setColumnWidth(dobCol, 112);
    m_table->setColumnWidth(heightCol, 84);
    m_table->setColumnWidth(weightCol, 84);
    m_table->setColumnWidth(mobileCol, 122);
    tableLayout->addWidget(m_table, 1);

    auto *statusRow = new QHBoxLayout;
    statusRow->setContentsMargins(16, 9, 16, 10);
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName(QStringLiteral("StatusLabel"));
    statusRow->addWidget(m_statusLabel, 1);
    auto *dbLabel = new QLabel(QStringLiteral("SQLite · %1")
                                   .arg(QFileInfo(DatabaseManager::instance().databasePath()).fileName()));
    dbLabel->setObjectName(QStringLiteral("HintLabel"));
    statusRow->addWidget(dbLabel);
    tableLayout->addLayout(statusRow);
    root->addWidget(tableCard, 1);

    connect(addButton, &QPushButton::clicked, this, &PatientPage::addPatient);
    connect(editButton, &QPushButton::clicked, this, &PatientPage::editPatient);
    connect(deleteButton, &QPushButton::clicked, this, &PatientPage::deletePatient);
    connect(refreshButton, &QPushButton::clicked, this, &PatientPage::refresh);
    connect(searchButton, &QPushButton::clicked, this, &PatientPage::applySearch);
    connect(resetButton, &QPushButton::clicked, this, &PatientPage::clearSearch);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PatientPage::applySearch);
    connect(m_searchField, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PatientPage::applySearch);
    connect(m_table, &QTableView::doubleClicked, this, &PatientPage::onDoubleClicked);

    refresh();
}

void PatientPage::setUser(const UserInfo &user)
{
    m_user = user;
}

void PatientPage::updateStatus()
{
    const int count = m_proxy->rowCount();
    m_countLabel->setText(QStringLiteral("共 %1 条记录").arg(count));
    const QString keyword = m_searchEdit->text().trimmed();
    if (count == 0 && !keyword.isEmpty()) {
        setStatus(QStringLiteral("未找到符合条件的患者，请调整查询条件"), QStringLiteral("#C0392B"));
    } else if (count == 0) {
        setStatus(QStringLiteral("暂无患者记录，点击“新增患者”开始建档"), QStringLiteral("#B7791F"));
    } else if (!keyword.isEmpty()) {
        setStatus(QStringLiteral("查询完成，显示 %1 条匹配记录").arg(count), QStringLiteral("#1F6F5E"));
    } else {
        setStatus(QStringLiteral("数据已与数据库同步，双击表格行可直接编辑"), QStringLiteral("#7A8A99"));
    }
}

void PatientPage::setStatus(const QString &text, const QString &color)
{
    m_statusLabel->setText(text);
    m_statusLabel->setStyleSheet(QStringLiteral("color:%1;").arg(color));
}

void PatientPage::refresh()
{
    m_model->select();
    updateStatus();
}

void PatientPage::applySearch()
{
    const QString keyword = m_searchEdit->text().trimmed();
    int column = -1;
    switch (m_searchField->currentIndex()) {
    case 1: column = m_model->columnIndex(QStringLiteral("name")); break;
    case 2: column = m_model->columnIndex(QStringLiteral("id_card")); break;
    case 3: column = m_model->columnIndex(QStringLiteral("mobile")); break;
    default: column = -1; break;
    }
    m_proxy->setFilterKeyColumn(column);
    m_proxy->setFilterFixedString(keyword);
    updateStatus();
}

void PatientPage::setSearchText(const QString &text)
{
    m_searchEdit->setText(text);
    applySearch();
}

void PatientPage::clearSearch()
{
    m_searchField->setCurrentIndex(0);
    m_searchEdit->clear();
    m_proxy->setFilterFixedString(QString());
    updateStatus();
}

void PatientPage::selectRow(int proxyRow)
{
    if (proxyRow < 0 || proxyRow >= m_proxy->rowCount()) {
        return;
    }
    const QModelIndex index = m_proxy->index(proxyRow, 0);
    m_table->setCurrentIndex(index);
    m_table->selectRow(proxyRow);
}

Patient PatientPage::selectedPatient() const
{
    const QModelIndex index = m_table->currentIndex();
    if (!index.isValid()) {
        return {};
    }
    const QModelIndex source = m_proxy->mapToSource(index);
    return m_model->patientAt(source.row());
}

void PatientPage::logEvent(const QString &event)
{
    const QString username = m_user.username.isEmpty() ? QStringLiteral("system") : m_user.username;
    DatabaseManager::instance().logEvent(username, event);
}

void PatientPage::addPatient()
{
    PatientDialog dialog(this, m_model);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    QString error;
    const Patient patient = dialog.patient();
    if (!m_model->insertPatient(patient, &error)) {
        QMessageBox::critical(this, QStringLiteral("新增失败"), error);
        return;
    }
    logEvent(QStringLiteral("新增患者：%1（%2）").arg(patient.name, patient.idCard));
    clearSearch();
    refresh();
    setStatus(QStringLiteral("新增成功：%1 已写入数据库").arg(patient.name), QStringLiteral("#1F6F5E"));
}

void PatientPage::editPatient()
{
    const Patient current = selectedPatient();
    if (current.id <= 0) {
        setStatus(QStringLiteral("请先选择需要编辑的患者记录"), QStringLiteral("#B7791F"));
        return;
    }
    PatientDialog dialog(this, m_model, &current);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    QString error;
    const Patient updated = dialog.patient();
    if (!m_model->updatePatient(updated, &error)) {
        QMessageBox::critical(this, QStringLiteral("修改失败"), error);
        return;
    }
    logEvent(QStringLiteral("修改患者：%1（%2）").arg(updated.name, updated.idCard));
    refresh();
    setStatus(QStringLiteral("修改成功：%1 的数据已更新并刷新列表").arg(updated.name),
              QStringLiteral("#1F6F5E"));
}

void PatientPage::deletePatient()
{
    const Patient current = selectedPatient();
    if (current.id <= 0) {
        setStatus(QStringLiteral("请先选择需要删除的患者记录"), QStringLiteral("#B7791F"));
        return;
    }
    ConfirmDialog dialog(this, current.name);
    if (dialog.exec() != QDialog::Accepted || !dialog.confirmed()) {
        return;
    }
    QString error;
    if (!m_model->removePatient(current.id, &error)) {
        QMessageBox::critical(this, QStringLiteral("删除失败"), error);
        return;
    }
    logEvent(QStringLiteral("删除患者：%1（%2）").arg(current.name, current.idCard));
    refresh();
    setStatus(QStringLiteral("已删除患者「%1」，数据库与列表已同步").arg(current.name),
              QStringLiteral("#1F6F5E"));
}

void PatientPage::onDoubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        editPatient();
    }
}

SimpleTablePage::SimpleTablePage(const QString &title, const QString &subtitle,
                                 const QString &query, QWidget *parent)
    : QWidget(parent)
    , m_query(query)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 22, 24, 18);
    root->setSpacing(14);

    auto *titleRow = new QHBoxLayout;
    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(4);
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setObjectName(QStringLiteral("PageSubtitle"));
    titleBox->addWidget(titleLabel);
    titleBox->addWidget(subtitleLabel);
    titleRow->addLayout(titleBox);
    titleRow->addStretch();
    auto *refreshButton = new QPushButton(QStringLiteral("刷新"));
    refreshButton->setObjectName(QStringLiteral("GhostButton"));
    titleRow->addWidget(refreshButton, 0, Qt::AlignBottom);
    root->addLayout(titleRow);

    auto *card = makeCard();
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    m_model = new QSqlQueryModel(this);
    m_table = new QTableView;
    m_table->setObjectName(QStringLiteral("simpleTable"));
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(38);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    cardLayout->addWidget(m_table, 1);

    auto *statusRow = new QHBoxLayout;
    statusRow->setContentsMargins(16, 9, 16, 10);
    m_status = new QLabel;
    m_status->setObjectName(QStringLiteral("StatusLabel"));
    statusRow->addWidget(m_status, 1);
    cardLayout->addLayout(statusRow);
    root->addWidget(card, 1);

    connect(refreshButton, &QPushButton::clicked, this, &SimpleTablePage::refresh);
    refresh();
}

void SimpleTablePage::refresh()
{
    m_model->setQuery(m_query, DatabaseManager::instance().database());
    if (m_model->lastError().isValid()) {
        m_status->setText(QStringLiteral("查询失败：%1").arg(m_model->lastError().text()));
        m_status->setStyleSheet(QStringLiteral("color:#C0392B;"));
        return;
    }
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_status->setText(QStringLiteral("共 %1 条记录").arg(m_model->rowCount()));
    m_status->setStyleSheet(QStringLiteral("color:#7A8A99;"));
}

WorkspacePage::WorkspacePage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("WorkspacePage"));
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *header = new QFrame;
    header->setObjectName(QStringLiteral("HeaderBar"));
    header->setFixedHeight(78);
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 12, 24, 12);
    headerLayout->setSpacing(12);

    auto *brand = new QVBoxLayout;
    brand->setSpacing(2);
    auto *title = new QLabel(QStringLiteral("医院诊疗信息管理系统"));
    title->setObjectName(QStringLiteral("AppTitle"));
    auto *subtitle = new QLabel(QStringLiteral("Patient Management Console · Qt 6 / SQLite"));
    subtitle->setObjectName(QStringLiteral("AppSubtitle"));
    brand->addWidget(title);
    brand->addWidget(subtitle);
    headerLayout->addLayout(brand);
    headerLayout->addStretch();

    m_userLabel = new QLabel;
    m_userLabel->setObjectName(QStringLiteral("UserBadge"));
    headerLayout->addWidget(m_userLabel);
    auto *logout = new QPushButton(QStringLiteral("退出登录"));
    logout->setObjectName(QStringLiteral("GhostButton"));
    headerLayout->addWidget(logout);
    root->addWidget(header);

    auto *body = new QHBoxLayout;
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);

    auto *sidebar = new QFrame;
    sidebar->setObjectName(QStringLiteral("Sidebar"));
    sidebar->setFixedWidth(212);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 20, 16, 18);
    sideLayout->setSpacing(8);

    auto *section = new QLabel(QStringLiteral("业务导航"));
    section->setObjectName(QStringLiteral("SidebarSection"));
    sideLayout->addWidget(section);
    sideLayout->addSpacing(4);

    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    const QStringList navTexts = {
        QStringLiteral("患者管理"), QStringLiteral("医生管理"),
        QStringLiteral("科室管理"), QStringLiteral("操作日志")
    };
    for (int i = 0; i < navTexts.size(); ++i) {
        auto *button = new QPushButton(navTexts.at(i));
        button->setObjectName(QStringLiteral("NavButton"));
        button->setCheckable(true);
        button->setMinimumHeight(42);
        group->addButton(button, i);
        sideLayout->addWidget(button);
        m_navButtons.append(button);
    }
    m_navButtons.first()->setChecked(true);
    sideLayout->addStretch();

    auto *developer = new QLabel(
        QStringLiteral("开发者：%1\n学号：%2").arg(AppConfig::kStudentName, AppConfig::kStudentId));
    developer->setObjectName(QStringLiteral("DeveloperLabel"));
    developer->setWordWrap(true);
    sideLayout->addWidget(developer);
    body->addWidget(sidebar);

    auto *content = new QWidget;
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    m_stack = new QStackedWidget;
    m_patientPage = new PatientPage;
    m_doctorPage = new SimpleTablePage(
        QStringLiteral("医生信息"),
        QStringLiteral("医生档案与所属科室信息（只读视图，数据来源于 doctor / department 表）"),
        QStringLiteral("SELECT d.employee_no AS 工号, d.name AS 姓名, dep.name AS 科室, "
                       "d.title AS 职称 FROM doctor d LEFT JOIN department dep "
                       "ON d.department_id = dep.id ORDER BY d.id"));
    m_departmentPage = new SimpleTablePage(
        QStringLiteral("科室信息"),
        QStringLiteral("科室列表及医生人数统计（只读视图）"),
        QStringLiteral("SELECT dep.name AS 科室, COUNT(d.id) AS 医生人数 "
                       "FROM department dep LEFT JOIN doctor d ON d.department_id = dep.id "
                       "GROUP BY dep.id ORDER BY dep.id"));
    m_logPage = new SimpleTablePage(
        QStringLiteral("操作日志"),
        QStringLiteral("记录登录、查询、新增、修改、删除等关键操作，便于追溯数据变化"),
        QStringLiteral("SELECT created_at AS 时间, username AS 用户, event AS 操作 "
                       "FROM history ORDER BY id DESC LIMIT 100"));
    m_stack->addWidget(m_patientPage);
    m_stack->addWidget(m_doctorPage);
    m_stack->addWidget(m_departmentPage);
    m_stack->addWidget(m_logPage);
    contentLayout->addWidget(m_stack);
    body->addWidget(content, 1);
    root->addLayout(body, 1);

    connect(group, QOverload<int>::of(&QButtonGroup::idClicked), this, &WorkspacePage::showPage);
    connect(logout, &QPushButton::clicked, this, &WorkspacePage::logoutRequested);
}

void WorkspacePage::setUser(const UserInfo &user)
{
    m_userLabel->setText(QStringLiteral("%1 · %2").arg(user.fullname, user.role));
    m_patientPage->setUser(user);
}

void WorkspacePage::refreshAll()
{
    m_patientPage->refresh();
    m_doctorPage->refresh();
    m_departmentPage->refresh();
    m_logPage->refresh();
}

void WorkspacePage::showPage(int index)
{
    if (index < 0 || index >= m_stack->count()) {
        return;
    }
    m_stack->setCurrentIndex(index);
    if (index >= 0 && index < m_navButtons.size()) {
        m_navButtons.at(index)->setChecked(true);
    }
    if (index == 0) m_patientPage->refresh();
    if (index == 1) m_doctorPage->refresh();
    if (index == 2) m_departmentPage->refresh();
    if (index == 3) m_logPage->refresh();
}

int WorkspacePage::currentPage() const
{
    return m_stack->currentIndex();
}
