#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> // <--- Додали цей інклуд для роботи з контекстом
#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Створюємо та підключаємо базу даних
    DatabaseManager dbManager;
    dbManager.connectToDatabase();

    QQmlApplicationEngine engine;

    // <--- НАЙВАЖЛИВІШИЙ РЯДОК: передаємо об'єкт dbManager в QML під іменем "myDbManager"
    engine.rootContext()->setContextProperty("myDbManager", &dbManager);

    // Сучасний спосіб завантаження QML у Qt 6
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("NotesSystem", "Main");

    return app.exec();
}
