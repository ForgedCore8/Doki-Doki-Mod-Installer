#include "MainWindow.h"
#include "DimmingOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QApplication>
#include <QPainter>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    initUI();
    connect(signalManager, &SignalManager::consoleUpdate, this, &MainWindow::appendToConsole);
    connect(signalManager, &SignalManager::progressUpdate, this, &MainWindow::updateProgressBar);
    connect(signalManager, &SignalManager::criticalMessagebox, this, &MainWindow::criticalMessageBox);
    connect(signalManager, &SignalManager::infoMessagebox, this, &MainWindow::infoMessageBox);
}

MainWindow::~MainWindow() {}

void MainWindow::initUI() {
    this->setWindowTitle("DDLC Mod Installer");
    this->resize(1200, 600);

    // Setup shortcut for reloading background
    QShortcut* reloadShortcut = new QShortcut(QKeySequence("Ctrl+R"), this);
    QObject::connect(reloadShortcut, &QShortcut::activated, this, &MainWindow::loadRandomBackground);

    // Setup central widget and layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // Setup for Zip File selection
    QHBoxLayout* zipLayout = new QHBoxLayout();
    QLabel* zipLabel = new QLabel("Zip File:");
    QLineEdit* zipEntry = new QLineEdit();
    QPushButton* zipBrowseButton = new QPushButton("Browse");
    zipLayout->addWidget(zipLabel);
    zipLayout->addWidget(zipEntry);
    zipLayout->addWidget(zipBrowseButton);
    layout->addLayout(zipLayout);

    // Setup for Game Directory selection
    QHBoxLayout* gamePathLayout = new QHBoxLayout();
    QLabel* gamePathLabel = new QLabel("Game Directory:");
    QLineEdit* gamePathEntry = new QLineEdit();
    QPushButton* gamePathBrowseButton = new QPushButton("Browse");
    QPushButton* autoButton = new QPushButton("Auto");
    gamePathLayout->addWidget(gamePathLabel);
    gamePathLayout->addWidget(gamePathEntry);
    gamePathLayout->addWidget(gamePathBrowseButton);
    gamePathLayout->addWidget(autoButton);
    layout->addLayout(gamePathLayout);

    // Setup for Install Mod to Separate Directory checkbox
    QCheckBox* newDirCheckbox = new QCheckBox("Install Mod to Separate Directory");
    layout->addWidget(newDirCheckbox);

    // Setup for Mod Path (Initially Hidden)
    QHBoxLayout* modPathLayout = new QHBoxLayout();
    QLabel* modPathLabel = new QLabel("Mod Path:");
    QLineEdit* modPathEntry = new QLineEdit();
    QPushButton* modPathBrowseButton = new QPushButton("Browse");
    modPathLayout->addWidget(modPathLabel);
    modPathLayout->addWidget(modPathEntry);
    modPathLayout->addWidget(modPathBrowseButton);
    layout->addLayout(modPathLayout);
    modPathLabel->setVisible(false);
    modPathEntry->setVisible(false);
    modPathBrowseButton->setVisible(false);

    // Setup for Install and Delete buttons
    QPushButton* installButton = new QPushButton("Install Mod");
    QPushButton* deleteButton = new QPushButton("Delete DDLC");
    layout->addWidget(installButton);
    layout->addWidget(deleteButton);

    // Setup for Console Output
    QLabel* consoleLabel = new QLabel("Console Output:");
    QTextEdit* consoleOutput = new QTextEdit();
    consoleOutput->setReadOnly(true);
    layout->addWidget(consoleLabel);
    layout->addWidget(consoleOutput);

    // Setup for Progress Bar (Initially Hidden)
    QProgressBar* progressBar = new QProgressBar();
    progressBar->setVisible(false);
    layout->addWidget(progressBar);

    // Finalize setting the central widget
    this->setCentralWidget(centralWidget);
    applyStyles();

    connect(zipBrowseButton, &QPushButton::clicked, this, [this]() { this->browsePath(zipEntry, false); });
    connect(gamePathBrowseButton, &QPushButton::clicked, this, [this]() { this->browsePath(gamePathEntry, true); });
    connect(autoButton, &QPushButton::clicked, this, &MainWindow::autoToggle);
    connect(newDirCheckbox, &QCheckBox::stateChanged, this, [this](int state) { utils::checkChanged(state, this); });
    connect(modPathBrowseButton, &QPushButton::clicked, this, [this]() { this->browsePath(modPathEntry, true); });
    connect(processButton, &QPushButton::clicked, this, &MainWindow::onButtonClick);
    connect(deleteButton, &QPushButton::clicked, this, [this]() { utils::deleteDDLC(gamePathEntry->text(), this); });
}

void MainWindow::updateProgressBar(float value) {
    progressBar->setValue(value);
    QApplication::processEvents();
}

void MainWindow::appendToConsole(const QString& message) {
    consoleOutput->setReadOnly(false);
    consoleOutput->append(message);
    // Ensure the cursor is at the end and the view scrolls to the bottom
    QTextCursor cursor = consoleOutput->textCursor();
    cursor.movePosition(QTextCursor::End);
    consoleOutput->setTextCursor(cursor);
    consoleOutput->setReadOnly(true);
    QApplication::processEvents();
}

void MainWindow::onButtonClick() {
    qDebug() << "Button clicked";
    // Gather input data from UI elements
    QString zipPath = zipEntry->text();
    QString gamePath = gamePathEntry->text();
    QString modPath = newDirCheckbox->isChecked() ? modPathEntry->text() : QString();

    // Perform input validation
    if (zipPath.isEmpty() || gamePath.isEmpty()) {
        QMessageBox::critical(this, "Error", "Please specify both the ZIP file and the game directory.");
        return;
    }

    if (newDirCheckbox->isChecked() && modPath.isEmpty()) {
        QMessageBox::critical(this, "Error", "Please specify the mod directory.");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    utils::showProgressBar(this); 
    utils::disableUIElements(this); 

    // Initialize and start the installation thread
    installThread = new InstallThread(zipPath, gamePath, modPath, this);
    connect(installThread, &InstallThread::finished, this, &MainWindow::threadFinished);
    installThread->start();
}

void MainWindow::threadFinished() {
    QApplication::restoreOverrideCursor();
    utils::enableUIElements(this);
}

void MainWindow::loadRandomBackground() {
    QDir directory(":/assets/backgrounds");
    QStringList images = directory.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files);
    if (!images.isEmpty()) {
        int index = QRandomGenerator::global()->bounded(images.size());
        QString imagePath = directory.filePath(images.at(index));
        backgroundPixmap.load(imagePath);
        update(); // Trigger a repaint to show the new background
    }
    else {
        qDebug() << "No image found or the 'backgrounds' directory is missing.";
    }
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QMainWindow::paintEvent(event); // Call base class paint event
    if (!backgroundPixmap.isNull()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Calculate the scaled pixmap size while maintaining aspect ratio
        QPixmap scaledPixmap = backgroundPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // Calculate top-left coordinates to center the pixmap in the window
        int startX = (this->width() - scaledPixmap.width()) / 2;
        int startY = (this->height() - scaledPixmap.height()) / 2;

        // Draw the pixmap at the calculated position
        painter.drawPixmap(startX, startY, scaledPixmap);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    if (overlay) {
        overlay->resize(event->size());
    }
    // Adjust additional UI elements as necessary
    QMainWindow::resizeEvent(event); // Ensure base class resize event is called
}

void MainWindow::browsePath(QLineEdit* entry, bool isFolder) {
    QString filename;
    if (isFolder) {
        filename = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
    }
    else {
        filename = QFileDialog::getOpenFileName(this, tr("Select File")).first;
    }

    if (!filename.isEmpty()) {
        entry->setText(filename);
        appendToConsole(isFolder ? "Directory selected: " + filename : "File selected: " + filename);
    }
}

void MainWindow::autoToggle() {
    QString gamePath = utils::findGameDirectory(); // Ensure this function is implemented in C++
    QString currentPath = gamePathEntry->text();

    if (currentPath.compare(gamePath, Qt::CaseInsensitive) == 0) {
        gamePathEntry->clear();
        appendToConsole("Auto detection disabled.");
    }
    else {
        gamePathEntry->setText(gamePath);
        appendToConsole("Auto detection enabled.");
    }
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

void MainWindow::infoMessageBox(const QString& title, const QString& message) {
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

void MainWindow::applyStyles() {
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
