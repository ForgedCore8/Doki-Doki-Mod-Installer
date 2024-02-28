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

class MainWindow : public QMainWindow
{
    Q_OBJECT

    // Background handling
    QPixmap background_pixmap;

    void initUI();
    void applyStyles();

    // Utility functions
    void threadFinished(); // Slot to handle thread completion

public:
    // UI Components
    DimmingOverlay* overlay;
    QLineEdit* zip_entry;
    QPushButton* zip_browse_button;
    QLineEdit* game_path_entry;
    QPushButton* game_path_browse_button;
    QPushButton* auto_button;
    QLineEdit* mod_path_entry;
    QCheckBox* new_dir_check_box;

    QLabel* mod_path_label;
    QPushButton* mod_path_browse_button;
    QPushButton* process_button;
    QPushButton* delete_button;
    
    QProgressBar* progress_bar;
    QTextEdit* console_output;
    QShortcut* reload_shortcut;

    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void loadRandomBackground();
    void updateProgressBar(float value);
    void appendToConsole(const QString &message);
    void onButtonClick();
    void browsePath(QLineEdit *entry, bool isFolder);
    void autoToggle();
    void criticalMessageBox(const QString &title, const QString &message);
    void infoMessageBox(const QString &title, const QString &message);
};

#endif // MAINWINDOW_H

