#include "capture.h"
#include "database.h"
#include "dialogs.h"
#include "mainwindow.h"
#include "selftest.h"
#include "theme.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("HospitalQt"));
    QApplication::setApplicationVersion(QStringLiteral("1.0"));
    QApplication::setStyle(QStringLiteral("Fusion"));
    app.setStyleSheet(appStyleSheet());

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("医院诊疗信息管理系统 - Qt SQL / Model-View 实验"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption dbOption(
        QStringList() << QStringLiteral("db"),
        QStringLiteral("SQLite 数据库文件路径"), QStringLiteral("path"));
    QCommandLineOption captureOption(
        QStringList() << QStringLiteral("capture"),
        QStringLiteral("生成实验截图到指定目录"), QStringLiteral("directory"));
    QCommandLineOption selfTestOption(
        QStringList() << QStringLiteral("selftest"),
        QStringLiteral("运行数据库与模型自测"));
    parser.addOption(dbOption);
    parser.addOption(captureOption);
    parser.addOption(selfTestOption);
    parser.process(app);

    QString dbPath;
    if (parser.isSet(dbOption)) {
        dbPath = parser.value(dbOption);
    } else {
        dbPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("hospital.db"));
    }

    if (parser.isSet(selfTestOption)) {
        return runSelfTests(dbPath);
    }

    if (!DatabaseManager::instance().open(dbPath)) {
        ErrorDialog dialog(nullptr, QStringLiteral("数据库连接失败"),
                           QStringLiteral("系统无法打开数据库文件，程序不能继续运行。请检查数据库路径、文件权限或磁盘空间。"),
                           DatabaseManager::instance().lastError());
        dialog.exec();
        return 1;
    }

    MainWindow window;
    window.show();

    if (parser.isSet(captureOption)) {
        return runCapture(window, parser.value(captureOption));
    }

    return app.exec();
}
