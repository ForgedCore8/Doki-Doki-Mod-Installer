#ifndef UTILS_H
#define UTILS_H

// ddmi.
#include "mainwindow.h"

// qt.
#include <QString>

// stdlib.
#include <filesystem>

namespace utils {
    extern int calculateDirectorySize(std::filesystem::path path);
    extern void deleteDirectoryWithProgress(QString path, int total_size, int deleted_size= 0);
    extern int yesnoMessageBox(MainWindow* main_window, QString title, QString message);
    extern void deleteDDLC(const QString& game_path, MainWindow* main_window);
    extern std::filesystem::path getSteamPath();
    extern QStringList parseVdfForPaths(QString vdf_path, QStringList game_ids= {"698780"});
    extern int calculateTotalSize(QString zip_path);
    extern QString findGameDirectory();
    extern void mergeDirectories(const QString& src, QString dst, long long processed_size, const QString& destinationPath, long long total_size);
    extern void overwriteFile(const QString& src, QString dst, long long& processed_size, const QString& destinationPath, qint64 total_size);
    extern void processFiles(MainWindow* main_window, QString zip_path, QString game_path, QString seperate_mod_path= "");
    extern void copyGameFiles(QString game_path, QString destination_path, int processed_size, int total_size);
    extern bool processExtractedFiles(QString extract_path, QString game_path, int processed_size, int total_size, QString destination_path="");
    void checkChanged(int state, MainWindow* main_window);

    // ui functions.
    extern void enableUiElements(MainWindow* main_window);
    extern void disableUiElements(MainWindow* main_window);
    extern void showProgressBar(MainWindow* main_window);
}

#endif // UTILS_H
