// qt.
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QLineEdit>
#include <QFrame>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QRandomGenerator>

// ddmi.
#include "dimming_overlay.h"
#include "install_thread.h"
#include "mainwindow.h"
#include "signal_manager.h"
#include "utils.h"

SignalManager signal_manager;

DimmingOverlay::DimmingOverlay(QWidget *parent)
    : QWidget(parent), overlayColor(0, 0, 0, 180) // Semi-transparent black
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void DimmingOverlay::paintEvent(QPaintEvent * /* event */) {
    QPainter painter(this);
    painter.fillRect(rect(), overlayColor);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    initUI();
    connect(&signal_manager, &SignalManager::consoleUpdate, this, &MainWindow::appendToConsole);
    connect(&signal_manager, &SignalManager::progressUpdate, this, &MainWindow::updateProgressBar);
    connect(&signal_manager, &SignalManager::criticalMessagebox, this, &MainWindow::criticalMessageBox);
    connect(&signal_manager, &SignalManager::infoMessagebox, this, &MainWindow::infoMessageBox);
}

void MainWindow::initUI() {
    this->setWindowTitle("DDLC Mod Installer");
    this->resize(1200, 600);

    // Setup shortcut for reloading background
    QShortcut* reload_shortcut = new QShortcut(QKeySequence("Ctrl+R"), this);
    QObject::connect(reload_shortcut, &QShortcut::activated, this, &MainWindow::loadRandomBackground);

    // Setup central widget and layout
    QWidget *central_widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central_widget);

    overlay = new DimmingOverlay(central_widget);
    overlay->setGeometry(central_widget->rect()); // Cover the entire central widget

    // Setup for Zip File selection
    QHBoxLayout *zip_layout = new QHBoxLayout();
    QLabel* zip_Label = new QLabel("Zip File:");
    QLineEdit* zip_entry = new QLineEdit();
    QPushButton* zip_browse_button = new QPushButton("Browse");
    zip_layout->addWidget(zip_Label);
    zip_layout->addWidget(zip_entry);
    zip_layout->addWidget(zip_browse_button);
    layout->addLayout(zip_layout);

    // Setup for Game Directory selection
    QHBoxLayout* game_path_layout = new QHBoxLayout();
    QLabel* game_path_label = new QLabel("Game Directory:");
    QLineEdit* game_path_entry = new QLineEdit();
    QPushButton* gamePathBrowseButton = new QPushButton("Browse");
    QPushButton* auto_button = new QPushButton("Auto");
    game_path_layout->addWidget( game_path_label);
    game_path_layout->addWidget(game_path_entry);
    game_path_layout->addWidget(gamePathBrowseButton);
    game_path_layout->addWidget(auto_button);
    layout->addLayout(game_path_layout);

    // Setup for Install Mod to Separate Directory checkbox
    QCheckBox* new_dir_checkbox = new QCheckBox("Install Mod to Separate Directory");
    layout->addWidget(new_dir_checkbox);

    // Setup for Mod Path (Initially Hidden)
    QHBoxLayout *mod_path_layout = new QHBoxLayout();
    QLabel *mod_path_label = new QLabel("Mod Path:");
    QLineEdit *modPathEntry = new QLineEdit();
    QPushButton *modPathBrowseButton = new QPushButton("Browse");
    mod_path_layout->addWidget(mod_path_label);
    mod_path_layout->addWidget(modPathEntry);
    mod_path_layout->addWidget(modPathBrowseButton);
    layout->addLayout(mod_path_layout);
    mod_path_label->setVisible(false);
    modPathEntry->setVisible(false);
    modPathBrowseButton->setVisible(false);

    // Setup for Install and Delete buttons
    QPushButton *installButton = new QPushButton("Install Mod");
    QPushButton *deleteButton = new QPushButton("Delete DDLC");
    layout->addWidget(installButton);
    layout->addWidget(deleteButton);

    // Setup for Console Output
    QLabel *consoleLabel = new QLabel("Console Output:");
    QTextEdit *consoleOutput = new QTextEdit();
    consoleOutput->setReadOnly(true);
    layout->addWidget(consoleLabel);
    layout->addWidget(consoleOutput);

    // Setup for Progress Bar (Initially Hidden)
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setVisible(false);
    layout->addWidget(progressBar);

    // Finalize setting the central widget
    this->setCentralWidget(central_widget);
    applyStyles();

    connect(zip_browse_button, &QPushButton::clicked, this, [this, &zip_entry]() { this->browsePath(zip_entry, false); });
    connect(gamePathBrowseButton, &QPushButton::clicked, this, [this, &game_path_entry]() { this->browsePath(game_path_entry, true); });
    connect(auto_button, &QPushButton::clicked, this, &MainWindow::autoToggle);
    connect(new_dir_checkbox, &QCheckBox::stateChanged, this, [this](int state) { utils::checkChanged(state, this); });
    connect(modPathBrowseButton, &QPushButton::clicked, this, [this]() { this->browsePath(mod_path_entry, true); });
    connect(process_button, &QPushButton::clicked, this, &MainWindow::onButtonClick);
    connect(deleteButton, &QPushButton::clicked, this, [this, &game_path_entry]() { utils::deleteDDLC(game_path_entry->text(), this); });
}

void MainWindow::loadRandomBackground() {
    QDir directory(":/assets/backgrounds");
    QStringList images = directory.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg",
    QDir::Files);

    if (!images.isEmpty()) {
        int index = QRandomGenerator::global()->bounded(images.size());
        QString imagePath = directory.filePath(images.at(index));
        background_pixmap.load(imagePath);
        update(); // Trigger a repaint to show the new background
    }
    else {
        qDebug() << "No image found or the 'backgrounds' directory is missing.";
    }
}

void MainWindow::appendToConsole(const QString &message) {
    console_output->setReadOnly(false);
    console_output->append(message);
    // Ensure the cursor is at the end and the view scrolls to the bottom
    QTextCursor cursor = console_output->textCursor();
    cursor.movePosition(QTextCursor::End);
    console_output->setTextCursor(cursor);
    console_output->setReadOnly(true);
    QApplication::processEvents();
}

void MainWindow::onButtonClick() {
    qDebug() << "Button clicked";
    // Gather input data from UI elements
    QString zipPath = zip_entry->text();
    QString gamePath = game_path_entry->text();
    QString modPath = new_dir_check_box->isChecked() ? mod_path_entry->text() : QString();

    // Perform input validation
    if (zipPath.isEmpty() || gamePath.isEmpty()) {
        criticalMessageBox("Error", "Please specify both the ZIP file and the game directory.");
        return;
    }

    if (new_dir_check_box->isChecked() && modPath.isEmpty()) {
        criticalMessageBox("Error", "Please specify the mod directory.");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    utils::showProgressBar(this);
    utils::disableUiElements(this);

    // Initialize and start the installation thread
    InstallThread* install_thread = new InstallThread(zipPath, gamePath, modPath, this);
    connect(install_thread, &InstallThread::finished, this, &MainWindow::threadFinished);
    install_thread->start();
}

void MainWindow::applyStyles()
{
    QString stylesheet = R"(
        QWidget {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            color: #fff;
        }
        QLabel, QPushButton, QCheckBox {
            margin: 10px;
        }
        QLineEdit, QTextEdit {
            background-color: rgba(30,30,30,220);
            color: #fff;
            border-radius: 5px;
            padding: 10px;
            border: 1px solid #777;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border-radius: 5px;
            padding: 10px 15px;
            border: none;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QProgressBar {
            border: 2px solid #2196F3;
            border-radius: 5px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #2196F3;
            width: 20px; /* Used to demonstrate chunk effect */
        }
        QCheckBox::indicator {
            width: 25px;
            height: 25px;
        }
        QCheckBox::indicator:checked {
            image: url(:/assets/ui/checked.svg);
        }
        QCheckBox::indicator:unchecked {
            image: url(:/assets/ui/unchecked.svg);
        }
    )";

    this->setStyleSheet(stylesheet);
}

void MainWindow::updateProgressBar(float value) {
    progress_bar->setValue(value);
    QApplication::processEvents();
}

void MainWindow::browsePath(QLineEdit *entry, bool is_folder) {
    QString filename;

    if (is_folder) { filename = QFileDialog::getExistingDirectory(this, tr("Select Directory")); }
    else { filename = QFileDialog::getOpenFileName(this, tr("Select File")); }

    if (!filename.isEmpty()) {
        entry->setText(filename);
        appendToConsole(is_folder ? "Directory selected: " + filename : "File selected: " + filename);
    }
}

void MainWindow::autoToggle() {

    QString game_path = utils::findGameDirectory(); 
    QString currentPath = game_path_entry->text();

    if (currentPath.compare(game_path, Qt::CaseInsensitive) == 0) {
        game_path_entry->clear();
        appendToConsole("Auto detection disabled.");
    }
    else {
        game_path_entry->setText(game_path);
        appendToConsole("Auto detection enabled.");
    }
}

void MainWindow::threadFinished() {
    QApplication::restoreOverrideCursor();
    utils::enableUiElements(this);
}

void MainWindow::infoMessageBox(const QString &title, const QString &message) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setStyleSheet(R"(
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
    )");
    msgBox.exec();
}

void MainWindow::criticalMessageBox(const QString& title, const QString& message) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setStyleSheet(R"(
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
    )");
    msgBox.exec();
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
