#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>

class SignalManager : public QObject
{
    Q_OBJECT

public:
    explicit SignalManager(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void consoleUpdate(const QString &message);
    void progressUpdate(float progress);
    void criticalMessagebox(const QString &title, const QString &message);
    void infoMessagebox(const QString &title, const QString &message);
};

#endif // SIGNALMANAGER_H
