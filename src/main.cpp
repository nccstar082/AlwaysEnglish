#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include "IconProvider.h"
#include "LnkResolver.h"
#include "ControlInputLayout.h"
#include "GetActiveWindowPath.h"
#include "helper/SettingsHelper.h"
#include "utils.hpp"

void logMessage(const QString &msg) {
    QString log = QString("[%1] %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).arg(msg);
    qDebug().noquote() << log;
}

int main(int argc, char *argv[]) {
    logMessage("Starting AlwaysEnglish...");
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
    logMessage("QGuiApplication created");

    SettingsHelper::getInstance()->init(argv);
    logMessage("SettingsHelper initialized");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale: uiLanguages) {
        const QString baseName = "AlwaysEnglish_" + QLocale(locale).name();
        if (translator.load("./i18n/" + baseName)) {
            app.installTranslator(&translator);
            logMessage("Loaded translator: " + baseName);
            break;
        }
    }

    QQmlApplicationEngine engine;
    
    logMessage("Registering context properties...");
    engine.rootContext()->setContextProperty("iconProvider", IconProvider::getInstance());
    engine.rootContext()->setContextProperty("LnkResolver", LnkResolver::getInstance());
    engine.rootContext()->setContextProperty("ControlInputLayout", ControlInputLayout::getInstance());
    engine.rootContext()->setContextProperty("GetActiveWindowPath", GetActiveWindowPath::getInstance());
    engine.rootContext()->setContextProperty("SettingsHelper", SettingsHelper::getInstance());
    engine.rootContext()->setContextProperty("Utils", Utils::getInstance());
    logMessage("Context properties registered");

    const QUrl url(QStringLiteral("qrc:/qml/App.qml"));
    logMessage("Loading QML: " + url.toString());
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl) {
                    qDebug() << "ERROR: Failed to load QML:" << url.toString();
                    QCoreApplication::exit(-1);
                } else {
                    qDebug() << "QML object created successfully:" << objUrl.toString();
                }
            }, Qt::QueuedConnection);
    
    engine.load(url);
    logMessage("QML engine loaded");

    logMessage("Starting event loop...");
    return app.exec();
}
