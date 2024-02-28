#ifndef INSTALL_THREAD_H
#define INSTALL_THREAD_H

// qt.
#include <QString>
#include <QThread>

// ddmi.
#include "mainwindow.h"

class InstallThread : public QThread
{
    QString _zip_path;
    QString _game_path;
    QString _seperate_mod_path;
    MainWindow* _main_window;

public:
    InstallThread(QString zip_path, QString game_path, QString seperate_mod_path, MainWindow* main_window);

    void run();
};

#endif // INSTALL_THREAD_H
