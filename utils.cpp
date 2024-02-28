// os.
#include <windows.h>

// stdlib.
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

// qt.
#include <QDirIterator>
#include <QMessageBox>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QStringList>
#include <QCoreApplication>
#include <QDebug>

// ddmi.
#include "signal_manager.h"
#include "utils.h"

// zlib.
#include <zlib.h>


int utils::calculateDirectorySize(const std::filesystem::path path) {
    // Calculate the total size of all files in the directory.
    int total_size = 0;

    for(const auto& thing : std::filesystem::recursive_directory_iterator(path)) {
        if(std::filesystem::is_regular_file(thing)) { total_size += std::filesystem::file_size(thing.path()); }
    }

    return total_size;
}

void utils::deleteDirectoryWithProgress(QString path, int total_size, int deleted_size) {
    // Delete a directory and its contents, updating the progress bar.

    std::string std_path = path.toStdString();

    if(!std::filesystem::exists(std_path))
        return;

    for (const auto& it : std::filesystem::recursive_directory_iterator(std_path)) {
        if(std::filesystem::is_regular_file(it)) {
            int file_size = std::filesystem::file_size(it.path());
            std::filesystem::remove(it.path());
            deleted_size += file_size;

            emit signal_manager.progressUpdate(static_cast<int>((deleted_size / static_cast<double>(total_size)) * 100));
        }
    }

    // todo: fix for() implementation.
    for (const auto& it : std::filesystem::recursive_directory_iterator(path.toStdString(), std::filesystem::directory_options::skip_permission_denied)) {
        if (std::filesystem::is_directory(it.path()))
            std::filesystem::remove(it.path());
    }

    std::filesystem::remove(std_path);
    emit signal_manager.progressUpdate(100);
}

int utils::yesnoMessageBox(MainWindow* main_window, QString title, QString message) {
    QMessageBox msg_box = QMessageBox(main_window);
    msg_box.setIcon(QMessageBox::Question);
    msg_box.setWindowTitle(title);
    msg_box.setText(message);
    msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg_box.setDefaultButton(QMessageBox::No);
    QString stylesheet = R"(
        QMessageBox {
            background-color: #2b2b2b;
        }
        QLabel {
            color: #ffffff;
        }
        QPushButton {
            background-color: #4b4b4b;
            color: #ffffff;
            border-radius: 4px;
            padding: 5px;
        }
        QPushButton:hover {
            background-color: #5b5b5b;
        }
        QPushButton:pressed {
            background-color: #2b2b2b;
        }
    )";
    msg_box.setStyleSheet(stylesheet);

    return msg_box.exec();
}

void utils::deleteDDLC(const QString& game_path, MainWindow* main_window) {
    // Delete the DDLC Folder for a Clean Install.

    // Check if the path is empty.
    if (game_path.isEmpty()) {
        emit signal_manager.criticalMessagebox("Error", "Game directory is empty. Please specify a valid path.");
        return;
    }

    // Safeguard checks to ensure the directory is indeed for DDLC.
    QStringList valid_names = {"Doki Doki Literature Club"};
    bool is_valid_name = std::any_of(valid_names.begin(), valid_names.end(), [&](const QString& name){ return game_path.contains(name); });
    if (!is_valid_name) {
        emit signal_manager.criticalMessagebox("Error", "The specified directory does not appear to be a valid DDLC installation.");
        emit signal_manager.consoleUpdate("Error: Attempted to delete a non-DDLC directory.");
        return;
    }

    // Check for existence of expected game files as an extra precaution
    QStringList expectedFiles = {"DDLC.exe", "DDLC.sh", "game"};
    bool hasExpectedFiles = std::any_of(expectedFiles.begin(), expectedFiles.end(), [&](const QString& file){ return QFile::exists(QDir(game_path).absoluteFilePath(file)); });
    if (!hasExpectedFiles) {
        emit signal_manager.criticalMessagebox("Error", "The specified directory does not contain expected DDLC files.");
        emit signal_manager.consoleUpdate("Error: The specified directory lacks expected DDLC files.");
        return;
    }

    // Confirmation dialog
    QMessageBox::StandardButton confirm = QMessageBox::question(
        main_window,
        "Confirm Uninstall",
        "Are you sure you want to Uninstall DDLC? This action cannot be undone!",
        QMessageBox::Yes | QMessageBox::No
        );

    if (confirm == QMessageBox::Yes) {
        try {
            // Calculate total size for progress
            int totalSize = calculateDirectorySize(game_path.toStdString());

            // Show progress bar
            showProgressBar(main_window); // Initialize progress bar

            // Delete directory with progress
            deleteDirectoryWithProgress(game_path, totalSize);
            emit signal_manager.consoleUpdate("DDLC has been uninstalled successfully from: " + game_path);
            emit signal_manager.infoMessagebox("Uninstall Complete", "DDLC has been successfully uninstalled.");
            main_window->game_path_entry->clear();
        }
        catch (const std::exception& e) {
            emit signal_manager.consoleUpdate("Error during uninstallation: " + QString::fromStdString(e.what()));
            emit signal_manager.criticalMessagebox("Error", "Failed to uninstall DDLC. " + QString::fromStdString(e.what()));
        }
    }
    else {
        emit signal_manager.consoleUpdate("Uninstallation cancelled.");
    }
}

std::filesystem::path getSteamPath() {
    // Find Steam on the System.

    std::filesystem::path value;

    try {
        LPCWSTR key_path = L"SOFTWARE\\Valve\\Steam";
        HKEY key;

        // use 64 bit path version if user is running 64 bit os.
        if(QString(std::getenv("PROCESSOR_ARCHITECTURE")).endsWith("64"))
            key_path = L"SOFTWARE\\Wow6432Node\\Valve\\Steam";

        // key opened successfully.
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_path, 0, KEY_READ, &key) == ERROR_SUCCESS) {
            DWORD data_size;
            std::string data;

            // value read failed.
            if(RegQueryValueExW(key, L"InstallPath", nullptr, nullptr, reinterpret_cast<LPBYTE>(&data[0]), &data_size) != ERROR_SUCCESS) {
                RegCloseKey(key);
                throw std::runtime_error("Failed to read value of Registry Key: " + std::to_string(_wtoi(key_path)));
            }

            RegCloseKey(key);
        }
        else{ throw std::runtime_error("Failed to open Registry Key:" + std::to_string(_wtoi(key_path))); }

        emit signal_manager.consoleUpdate(QString("Steam Path Value: ") + value.string().data());
    }
    catch(std::exception e) {
        emit signal_manager.consoleUpdate(QString("Error accessing registry: ") + e.what());
    }

    return value;
}

QStringList utils::parseVdfForPaths(QString vdf_path, QStringList game_ids) {
    // Find the Game Path in the steam library.

    QStringList paths;

    try {
        std::ifstream file(vdf_path.toStdString());

        // couldn't open vdf path.
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open VDF file.");
        }

        std::string line;
        QString current_path;
        bool in_apps_block = false;

        while (std::getline(file, line)) {
            std::string stripped_line = line;

            if (stripped_line.find("\"path\"") != std::string::npos) {
                std::istringstream iss(stripped_line);
                std::string token;
                std::vector<std::string> tokens;

                while (std::getline(iss, token, '"')) { tokens.push_back(token); }

                if (tokens.size() >= 4) {
                    current_path = QString::fromStdString(tokens[3]);
                    current_path.replace("\\\\", "\\");
                }
            }

            if(stripped_line.find("\"apps\"") != std::string::npos) {
                in_apps_block = true;
            }
            else if(in_apps_block) {
                if (std::any_of(
                        game_ids.begin(),
                        game_ids.end(),
                        [&](const std::string& id) {
                            return stripped_line.find(id) != std::string::npos;
                        })
                    ) {
                    if (
                        !current_path.isEmpty() &&
                        std::find(paths.begin(), paths.end(), current_path) == paths.end()&&
                        std::filesystem::exists(current_path.toStdString())
                        ) {
                        paths.push_back(current_path);
                        emit signal_manager.consoleUpdate(QString("Found Steam library with game: ") + current_path);
                        current_path = "";
                        in_apps_block = false;
                    }
                } else if (stripped_line.find("}") != std::string::npos) {
                    in_apps_block = false;
                    current_path = ""; // Reset current path after exiting an apps block
                }
            }
        }
    }
    catch (const std::exception& e) {
        emit signal_manager.consoleUpdate(QString("Error parsing VDF: ") + e.what());
    }

    QString vdf_paths_string;
    for(const auto& path : paths) { vdf_paths_string + path + "; "; }

    emit signal_manager.consoleUpdate("VDF Paths: " + vdf_paths_string);

    return paths;
}

int utils::calculateTotalSize(QString zip_path) {
    // Get total size of zip.

    int total_size = 0;

    gzFile zip_file = gzopen(zip_path.toStdString().data(), "rb");

    // zip file opened.
    if(!zip_file) {
        emit signal_manager.consoleUpdate("Error opening zip file: " + zip_path);
    }
    else {
        int byte;
        while((byte = gzgetc(zip_file)) != -1) { total_size += 1; }
    }

    gzclose(zip_file);

    return total_size;
}

QString utils::findGameDirectory() {
    QString steam_path = QString::fromStdString(getSteamPath().string());

    // got steam path.
    if(!steam_path.isEmpty()) {
        QString vdf_path = steam_path + "/steamapps/libraryfolders.vdf";
        QStringList library_paths = parseVdfForPaths(vdf_path);

        library_paths.insert(library_paths.begin(), steam_path);

        for (const auto& path : library_paths) {
            QString game_path = path + "/steamapps/common/Doki Doki Literature Club";

            QString message = QString("Game Path: %1").arg(game_path);
            emit signal_manager.consoleUpdate(message);

            // found game path.
            if (std::filesystem::exists(game_path.toStdString())) { return game_path; }
        }
    }
    return "Game directory not found automatically.";
}

void utils::mergeDirectories(const QString& src, QString dst, long long processed_size, const QString& destinationPath, long long total_size) {
    // Adjust destination path based on user choice
    dst = QDir(destinationPath).absoluteFilePath(dst);

    QDir source_dir(src);

    // source dir nonexistent.
    if (!source_dir.exists()) {
        emit signal_manager.consoleUpdate("Source directory does not exist: " + src);
        return;
    }

    // List all files and directories in the source directory recursively
    QStringList files = source_dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst);
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden;
    QDirIterator it(src, filters, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        QString relativePath = source_dir.relativeFilePath(filePath);
        QString destinationFilePath = QDir(dst).absoluteFilePath(relativePath);

        QFileInfo fileInfo(filePath);

        if (fileInfo.isDir()) {
            // Ensure the destination directory exists
            QDir().mkpath(destinationFilePath);
        }
        else {
            // Handle file copying
            qint64 fileSize = fileInfo.size();
            QFile destinationFile(destinationFilePath);


            if (destinationFile.exists()) {
                emit signal_manager.consoleUpdate(QString("Overwriting file: %1").arg(destinationFilePath));
            }
            else {
                emit signal_manager.consoleUpdate(QString("Copying file: %1").arg(destinationFilePath));
            }

            QFile::copy(filePath, destinationFilePath);

            processed_size += fileSize; // Update processed size

            double progressPercentage = static_cast<double>(processed_size) / total_size * 100.0;
            emit signal_manager.progressUpdate(progressPercentage); // Update progress bar
        }
    }
}

void utils::overwriteFile(const QString& src, QString dst, long long& processed_size, const QString& destinationPath, qint64 total_size) {
    // Adjust destination path
    dst = QDir(destinationPath).absoluteFilePath(dst);

    // Get file size
    QFileInfo srcInfo(src);
    qint64 fileSize = srcInfo.size();

    // Check if destination file exists and remove it
    QFile dstFile(dst);
    if (dstFile.exists()) {
        dstFile.remove();
        emit signal_manager.consoleUpdate(QString("Removed existing file: %1").arg(dst));
    }

    // Copy the file
    QFile::copy(src, dst);
    emit signal_manager.consoleUpdate(QString("Copied %1 to %2").arg(src, dst));

    // Update processed size and emit progress
    processed_size += fileSize;
    double progress = (static_cast<double>(processed_size) / total_size) * 100;
    emit signal_manager.progressUpdate(progress);
}

void utils::processFiles(MainWindow* main_window, QString zip_path, QString game_path, QString seperate_mod_path) {
    emit signal_manager.consoleUpdate(QString("Processing files from: %1 to %2").arg(zip_path, game_path));

    if (QFileInfo(zip_path).suffix() != ".zip") {
        emit signal_manager.consoleUpdate("Error: The provided path does not point to a zip file.");
        return;
    }

    QString destination_path = seperate_mod_path.isEmpty() ? game_path : seperate_mod_path;
    int gameDirSize = calculateDirectorySize(game_path.toStdString());
    int modFileSize = calculateTotalSize(zip_path);

    int total_size = gameDirSize + modFileSize;
    if (total_size <= 0) { total_size = 100; }

    bool openDir = false;

    try {
        // Initialize progress tracking (adjust as needed for C++)
        int processed_size = 0;

        // failed to extract zip.
        if (!extractZip(zip_path, destination_path)) {
            return;
        }

        emit signal_manager.consoleUpdate(QString("Extracted zip to: %1").arg(destination_path));

        // Copy game files (if separate mod path)
        if (!seperate_mod_path.isEmpty()) { copyGameFiles(game_path, seperate_mod_path, processed_size, total_size); }

        // Process extracted files (open directory if modified)
        openDir = processExtractedFiles(game_path, destination_path, total_size, processed_size);

    }
    catch (const std::exception &e) {
        emit signal_manager.consoleUpdate(QString("Error during processing: %1").arg(QString::fromStdString(e.what())));
        emit signal_manager.criticalMessagebox("Error", QString::fromStdString(e.what()));
    }

    emit signal_manager.progressUpdate(100.0f);
    enableUiElements(main_window);
    emit signal_manager.infoMessagebox("Process Completed", "All files have been processed successfully.");

    if(openDir) {
        destination_path = QFileInfo(destination_path).absoluteFilePath();

        if (QDir(destination_path).exists()) { QProcess::execute("explorer", QStringList(destination_path)); }
        else { emit signal_manager.criticalMessagebox("Error", "The specified path does not exist."); }
    }

    return;
}

// finish.
void copyGameFiles(QString game_path, QString destination_path, int processed_size, int total_size) {
    // Copy all game files to the destination directory and update progress.

    for(const auto& thing : std::filesystem::directory_iterator(game_path)) {
        std::filesystem::path src_path = std::filesystem::path(game_path) / item;
        std::filesystem::path dst_path = std::filesystem::path(destination_path) / item;

        if(src_path.is_directory()) {
            for() {

            }
        }
        else if(!dst_path.exists()) {

        }
    }

    emit SignalManager.consoleUpdate(QString("Copied game files to: " + destination_path));
}

// fin
bool utils::processExtractedFiles(QString extract_path, QString game_path, int processed_size, int total_size, QString destination_path="") {
    // Process Game files after zip extraction.
    bool open_dir = false;
    QStringList target_files = ["audio.rpa", "fonts.rpa", "images.rpa", "scripts.rpa"];
    QStringList target_dirs = ["game", "characters", "lib", "renpy"];
    QStringList executable_extensions = [".exe", ".bat", ".sh", ".py"];

    // Use game path if no separate mod path is provided
    if(!destination_path)
        destination_path = game_path;

    // Find the directory that contains any of the target directories or files
    for(const auto& thing : std::filesystem::recursive_directory_iterator(extract_path)) {
        if(thing.is_directory() && std::any_of(target_dirs.begin(), target_dirs.end(),[&thing](const std::string& dir) { return thing.path().filename() == dir; })) {
            baseDir = thing.path().string();
            break;
        }
    }

    // Process files and directories from the found base directory.
    for() {

    }

    return open_dir;
}

void utils::checkChanged(int state, MainWindow* main_window) {
    // When the Checkbox is changed, show or hide the mod path widgets.
    if(state == 2) {
        main_window->mod_path_label->setVisible(true);
        main_window->mod_path_entry->setVisible(true);
        main_window->mod_path_browse_button->setVisible(true);
    }
    else {
        main_window->mod_path_entry->clear();
        main_window->mod_path_label->setVisible(false);
        main_window->mod_path_entry->setVisible(false);
        main_window->mod_path_browse_button->setVisible(false);
    }
}

void utils::disableUiElements(MainWindow* main_window) {
    // Enable all UI elements.
    main_window->zip_browse_button->setEnabled(false);
    main_window->game_path_browse_button->setEnabled(false);
    main_window->auto_button->setEnabled(false);
    main_window->process_button->setEnabled(false);
    main_window->delete_button->setEnabled(false);
}

void utils::enableUiElements(MainWindow* main_window) {
    // Enable all UI elements.
    main_window->zip_browse_button->setEnabled(true);
    main_window->game_path_browse_button->setEnabled(true);
    main_window->auto_button->setEnabled(true);
    main_window->process_button->setEnabled(true);
    main_window->delete_button->setEnabled(true);
}

void utils::showProgressBar(MainWindow* main_window) {
    main_window->progress_bar->setValue(0);
    main_window->progress_bar->setVisible(true);
}
