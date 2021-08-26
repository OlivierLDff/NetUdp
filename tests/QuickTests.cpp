#include <NetUdp/NetUdp.hpp>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQuickTest/QtQuickTest>

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup()
    {
    }

public slots:
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        netudp::registerQmlTypes();
    }
};

QUICK_TEST_MAIN_WITH_SETUP(QuickTests, Setup)

#include "QuickTests.moc"
