#include <QGuiApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <cstring>
#include <iostream>
#include <string>
#include <QQmlContext>

#include "logic_2048_tmp.h"
#include "game_2048.h"
#include <QIcon>

int runBenchmarks();

int main(int argc, char* argv[])
{
    void getGPUInformation();
    getGPUInformation();

    if (argc >= 2 && std::string(argv[1]) == "--bench") {

        return runBenchmarks();
    }

    // 创建 QApplication 资源
    const QGuiApplication app(argc, argv);

    QGuiApplication::setWindowIcon(QIcon(":/icon/logo.png"));

    // 创建 Game 对象
    Game2048 game2048{nullptr};

    // 创建 QML 引擎
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("game2048", &game2048);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl)
        {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    return QGuiApplication::exec();
}
