#include "capture.h"

#include "database.h"
#include "dialogs.h"
#include "loginpage.h"
#include "mainwindow.h"
#include "patientmodel.h"
#include "workspace.h"

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QPixmap>
#include <QSqlQuery>
#include <QTimer>
#include <QDebug>

namespace {

void waitFor(int milliseconds)
{
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

void capture(QWidget *widget, const QString &directory, const QString &fileName)
{
    const QString path = QDir(directory).filePath(fileName);
    widget->repaint();
    QApplication::processEvents();
    const QPixmap pixmap = widget->grab();
    if (!pixmap.save(path, "PNG")) {
        qWarning() << "failed to save" << path;
    }
}

} // namespace

int runCapture(MainWindow &window, const QString &directory)
{
    if (!QDir().mkpath(directory)) {
        qCritical() << "cannot create capture directory" << directory;
        return 1;
    }

    window.show();
    waitFor(500);
    capture(&window, directory, QStringLiteral("01_login.png"));

    window.loginPage()->setCredentials(QStringLiteral("admin"), QStringLiteral("wrong"));
    window.loginPage()->submit();
    waitFor(300);
    capture(&window, directory, QStringLiteral("02_login_error.png"));

    window.loginPage()->setCredentials(QStringLiteral("admin"), QStringLiteral("123456"));
    window.loginPage()->submit();
    waitFor(700);
    capture(&window, directory, QStringLiteral("03_patient_list.png"));

    PatientPage *patientPage = window.workspace()->patientPage();
    patientPage->setSearchText(QStringLiteral("不存在的患者"));
    waitFor(300);
    capture(&window, directory, QStringLiteral("04_search_empty.png"));

    patientPage->clearSearch();
    patientPage->refresh();
    patientPage->selectRow(0);
    waitFor(200);
    const Patient selected = patientPage->selectedPatient();

    {
        PatientDialog dialog(&window, patientPage->model(), &selected);
        dialog.show();
        waitFor(350);
        capture(&dialog, directory, QStringLiteral("05_edit_dialog.png"));
        dialog.close();
    }

    {
        PatientDialog dialog(&window, patientPage->model(), nullptr);
        dialog.show();
        waitFor(350);
        dialog.setName(QString());
        dialog.setIdCard(QStringLiteral("440101200001010011"));
        dialog.setMobile(QStringLiteral("13800000000"));
        dialog.attemptSave();
        waitFor(250);
        capture(&dialog, directory, QStringLiteral("06_add_validation.png"));
        dialog.close();
    }

    {
        ConfirmDialog dialog(&window, selected.name);
        dialog.show();
        waitFor(250);
        capture(&dialog, directory, QStringLiteral("07_delete_confirm.png"));
        dialog.close();
    }

    // Exercise the same insert / update / delete path used by the UI so the
    // operation-log page contains real CRUD evidence, not just login entries.
    {
        Patient temp;
        temp.name = QStringLiteral("演示患者");
        temp.idCard = QStringLiteral("440101200001010099");
        temp.sex = QStringLiteral("女");
        temp.dob = QStringLiteral("2000-01-01");
        temp.mobile = QStringLiteral("13900000000");
        temp.height = 165.0;
        temp.weight = 55.0;
        temp.age = 25;
        QString error;
        if (patientPage->model()->insertPatient(temp, &error)) {
            DatabaseManager::instance().logEvent(QStringLiteral("admin"),
                                                 QStringLiteral("新增患者：演示患者（440101200001010099）"));
            QSqlQuery query(DatabaseManager::instance().database());
            query.prepare(QStringLiteral("SELECT id FROM patient WHERE id_card = ?"));
            query.addBindValue(temp.idCard);
            if (query.exec() && query.next()) {
                temp.id = query.value(0).toInt();
                temp.name = QStringLiteral("演示患者·修改");
                patientPage->model()->updatePatient(temp, &error);
                DatabaseManager::instance().logEvent(
                    QStringLiteral("admin"),
                    QStringLiteral("修改患者：演示患者·修改（440101200001010099）"));
                patientPage->model()->removePatient(temp.id, &error);
                DatabaseManager::instance().logEvent(
                    QStringLiteral("admin"),
                    QStringLiteral("删除患者：演示患者·修改（440101200001010099）"));
            }
            patientPage->refresh();
        }
    }

    window.workspace()->showPage(1);
    waitFor(350);
    capture(&window, directory, QStringLiteral("08_doctor_page.png"));

    window.workspace()->showPage(3);
    waitFor(350);
    capture(&window, directory, QStringLiteral("09_log_page.png"));

    {
        QString error;
        DatabaseManager::probe(QStringLiteral("Z:/no/such/directory/hospital.db"), &error);
        ErrorDialog dialog(&window, QStringLiteral("数据库连接失败"),
                           QStringLiteral("系统无法打开数据库文件，登录与患者数据功能不可用。请检查数据库路径、文件权限或磁盘空间。"),
                           error);
        dialog.show();
        waitFor(300);
        capture(&dialog, directory, QStringLiteral("10_db_error.png"));
        dialog.close();
    }

    qInfo() << "capture finished";
    return 0;
}
