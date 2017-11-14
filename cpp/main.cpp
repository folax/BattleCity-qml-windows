#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScopedPointer>
#include "./cpp/bcgame.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QScopedPointer<bcGame> bcG(new bcGame);
    //qmlRegisterType<QVector<TankObj*>>();

    QQmlApplicationEngine engine;
    bcGame::declareQML();

    engine.rootContext()->setContextProperty("cppObject", bcG.data());
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}

