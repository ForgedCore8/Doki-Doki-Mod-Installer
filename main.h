#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QBrush>
#include <QColor>
#include <QTextCursor>
#include <QKeyEvent>

class DimmingOverlay;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void loadRandomBackground();
    void updateProgressBar(float value);
    void appendToConsole(const QString& message);
    void onButtonInstallModClick();
    void onButtonDeleteDDLCClick();
    void browsePath(bool isFolder);
    void autoDetectGamePath();
    void criticalMessageBox(const QString& title, const QString& message);
    void infoMessageBox(const QString& title, const QString& message);

private:
    void initUI();
    void connectSignals();
    void applyStyles();

    // UI Components
    DimmingOverlay* overlay;
    QLineEdit* zipEntry;
    QLineEdit* gamePathEntry;
    QLineEdit* modPathEntry;
    QPushButton* zipBrowseButton;
    QPushButton* gamePathBrowseButton;
    QPushButton* modPathBrowseButton;
    QPushButton* processButton;
    QPushButton* deleteButton;
    QCheckBox* newDirCheckBox;
    QProgressBar* progressBar;
    QTextEdit* consoleOutput;
    QShortcut* reloadShortcut;

    // Utility functions
    void browsePath(QLineEdit* entry, bool isFolder);
    void threadFinished(); // Slot to handle thread completion

    // Background handling
    QPixmap backgroundPixmap;
};

#endif // MAINWINDOW_H
