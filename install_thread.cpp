// ddmi.
#include "install_thread.h"
#include "signal_manager.h"
#include "utils.h"

// stdlib.
#include <exception>

InstallThread::InstallThread(QString zip_path, QString game_path, QString seperate_mod_path, MainWindow* main_window) {
    this->_zip_path = zip_path;
    this->_game_path = game_path;
    this->_seperate_mod_path = seperate_mod_path;
    this->_main_window = main_window;
}

void InstallThread::run() {
    try {
        utils::processFiles(_main_window, _zip_path, _game_path, _seperate_mod_path);
    }
    catch (std::exception e) {
        emit signal_manager.consoleUpdate(QString("Error: ") + e.what());
    }
}
