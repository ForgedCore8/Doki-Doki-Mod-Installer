"""DDMI UI"""
import sys
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QVBoxLayout,
    QLabel,
    QMainWindow,
    QLineEdit,
    QFrame,
    QPushButton,
    QCheckBox,
    QTextEdit,
    QProgressBar,
    QHBoxLayout,
    QFileDialog,
    QMessageBox
    )
from PySide6.QtCore import Qt
from PySide6.QtGui import QPixmap, QPalette, QBrush, QColor, QTextCursor
import ddmi
from ddmi import InstallThread
from signal_manager import signal_manager


class DimmingOverlay(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setPalette(QColor(0, 0, 0, 180))  # Set the semi-transparent color
        self.setAutoFillBackground(True)
class MainWindow(QMainWindow):
    """The Main UI interface"""
    def __init__(self):
        super().__init__()
        self.init_ui()
        signal_manager.console_update.connect(self.append_to_console)
        signal_manager.progress_update.connect(self.update_progress_bar)


    def init_ui(self):
        """Initialize ui for the app"""
        self.setWindowTitle("DDLC Mod Installer")
        self.resize(800, 600)  # Adjust the size as needed


        # Set background image
        self.setAutoFillBackground(True)
        pixmap = QPixmap('.\\backgrounds\\natsuki.png')  # Ensure the image path is correct
        palette = QPalette()
        palette.setBrush(QPalette.Window, QBrush(pixmap))
        self.setPalette(palette)
        # Set a dummy central widget and apply the layout to it


        self.central_widget = QWidget(self)
        self.setCentralWidget(self.central_widget)

        #self.setWindowFlags(Qt.FramelessWindowHint)
        #self.frame_WINDOW_HINT = QFrame(self)
        #self.frame_WINDOW_HINT.setGeometry(0, 0, self.width(), 30)
        #self.frame_WINDOW_HINT.setStyleSheet("background-color: rgb(23, 39, 90);")
        #self.button_minimize = QPushButton("-", self.frame_WINDOW_HINT)
        #self.button_minimize.setGeometry(self.width() - 40, 0, 20, 20)
        #self.button_minimize.setStyleSheet("background-color: rgb(23, 39, 90); color: white; font-size: 10px; ")
        #self.button_close = QPushButton("X", self.frame_WINDOW_HINT)
        #self.button_close.setGeometry(self.width() - 20, 0, 20, 20)
        #self.button_close.setStyleSheet("background-color: rgb(23, 39, 90); color: white; font-size: 10px;")
        #self.button_minimize.clicked.connect(self.showMinimized)
        #self.button_close.clicked.connect(self.close)

        # Create the overlay widget and add it to the main window
        self.overlay = DimmingOverlay(self.central_widget)
        self.overlay.setGeometry(self.central_widget.rect())
        # Apply QSS styles
        self.apply_styles()

        layout = QVBoxLayout(self.central_widget)
        # Mod Zip Location
        zip_layout = QHBoxLayout()
        zip_label = QLabel("Zip File:")
        self.zip_entry = QLineEdit()
        self.zip_browse_button = QPushButton("Browse")

        zip_layout.addWidget(zip_label)
        zip_layout.addWidget(self.zip_entry)
        zip_layout.addWidget(self.zip_browse_button)
        layout.addLayout(zip_layout)

        # Game Path
        game_path_layout = QHBoxLayout()
        game_path_label = QLabel("Game Directory:")
        self.game_path_entry = QLineEdit()
        self.game_path_browse_button = QPushButton("Browse")

        self.auto_button = QPushButton("Auto")

        game_path_layout.addWidget(game_path_label)
        game_path_layout.addWidget(self.game_path_entry)
        game_path_layout.addWidget(self.game_path_browse_button)
        game_path_layout.addWidget(self.auto_button)
        layout.addLayout(game_path_layout)

        # Checkbox for separate directory
        self.newdir_checkbox = QCheckBox("Install Mod to Separate Directory")
        layout.addWidget(self.newdir_checkbox)

        # Mod Path (Initially Hidden)
        self.mod_path_group = QHBoxLayout()
        self.mod_path_label = QLabel("Mod Path:")
        self.mod_path_entry = QLineEdit()
        self.mod_path_browse_button = QPushButton("Browse")

        self.mod_path_group.addWidget(self.mod_path_label)
        self.mod_path_group.addWidget(self.mod_path_entry)
        self.mod_path_group.addWidget(self.mod_path_browse_button)
        layout.addLayout(self.mod_path_group)

        self.mod_path_label.setVisible(False)
        self.mod_path_entry.setVisible(False)
        self.mod_path_browse_button.setVisible(False)

        # _buttons for processing and deleting
        self.process_button = QPushButton("Install Mod")
        self.delete_button = QPushButton("Delete DDLC")
        layout.addWidget(self.process_button)
        layout.addWidget(self.delete_button)

        # Console Output
        console_label = QLabel("Console Output:")
        self.console_output = QTextEdit()
        self.console_output.setReadOnly(True)
        layout.addWidget(console_label)
        layout.addWidget(self.console_output)

        # Progress Bar
        self.progress_bar = QProgressBar()
        self.progress_bar.setVisible(False)
        layout.addWidget(self.progress_bar)

        self.setLayout(layout)

        self.zip_browse_button.clicked.connect(lambda: self.browse_path(self.zip_entry, False))
        self.game_path_browse_button.clicked.connect(lambda: self.browse_path(self.game_path_entry, True))
        self.auto_button.clicked.connect(lambda: self.auto_toggle())
        self.newdir_checkbox.stateChanged.connect(slot=lambda state: ddmi.check_changed(state, self))
        self.mod_path_browse_button.clicked.connect(lambda: self.browse_path(self.mod_path_entry, True))
        self.process_button.clicked.connect(slot=lambda: self.on_button_click())
        self.delete_button.clicked.connect(slot=lambda: ddmi.delete_ddlc(self.game_path_entry.text(), self))

    def start_installation_process(self):
        # Read paths from the UI
        zip_path = self.zip_entry.text()
        game_path = self.game_path_entry.text()
        separate_mod_path = self.mod_path_entry.text()  # Assume some logic to determine if this should be used

        # Check if paths are provided
        if not zip_path or not game_path:
            QMessageBox.warning(self, "Error", "Please specify both the ZIP file and the game directory.")
            return
        if self.newdir_checkbox.isChecked() and not separate_mod_path:  # Assuming you have logic to decide if this is required
            QMessageBox.warning(self, "Error", "Please specify the mod directory.")
            return

        # Create and start the InstallThread
        install_thread = InstallThread(zip_path, game_path, separate_mod_path, self)
        install_thread.start()

    def update_progress_bar(self, value):
        self.progress_bar.setValue(value)
        QApplication.processEvents()

    def append_to_console(self, message):
        self.console_output.setReadOnly(False)
        self.console_output.append(message)
        # Ensure the cursor is at the end and the view scrolls to the bottom
        cursor = self.console_output.textCursor()
        cursor.movePosition(QTextCursor.End)
        self.console_output.setTextCursor(cursor)
        self.console_output.setReadOnly(True)
        QApplication.processEvents()

    def on_button_click(self):
        print("button clicked")
        # Gather input data from UI elements
        zip_path = self.zip_entry.text()
        game_path = self.game_path_entry.text()
        mod_path = self.mod_path_entry.text() if self.newdir_checkbox.isChecked() else None

        # Perform input validation
        if not zip_path or not game_path:
            QMessageBox.critical(self, "Error", "Please specify both the ZIP file and the game directory.")
            return

        if self.newdir_checkbox.isChecked() and not mod_path:
            QMessageBox.critical(self, "Error", "Please specify the mod directory.")
            return

        QApplication.setOverrideCursor(Qt.WaitCursor)
        ddmi.show_progressbar(self)
        ddmi.disable_ui_elements(self)

        # Initialize and start the installation thread
        self.install_thread = InstallThread(zip_path, game_path, mod_path, self)
        self.install_thread.finished.connect(self.thread_finished)
        self.install_thread.start()

    def thread_finished(self):
        # Called when the thread finishes
        QApplication.restoreOverrideCursor()
        ddmi.enable_ui_elements(self)
        QMessageBox.information(self, "Done", "Installation complete.")

    def resizeEvent(self, event):
        # Resize the overlay widget to fill the entire window
        self.overlay.resize(event.size())
        #self.frame_WINDOW_HINT.setGeometry(0, 0, self.width(), 30)
        #self.button_minimize.setGeometry(self.width() - 42, 0, 20, 20)
        #self.button_close.setGeometry(self.width() - 21, 0, 20, 20)
        event.accept()

    def browse_path(self, entry, is_folder=False):
        if is_folder:
            filename = QFileDialog.getExistingDirectory(self, "Select Directory")
        else:
            filename, _ = QFileDialog.getOpenFileName(self, "Select File")

        if filename:
            entry.setText(filename)
            self.append_to_console(f"{'Directory' if is_folder else 'File'} selected: {filename}")

    def auto_toggle(self):
        game_path = ddmi.find_game_directory() 
        current_path = self.game_path_entry.text()

        if current_path.lower() == game_path.lower():
            self.game_path_entry.clear()
            self.append_to_console("Auto detection disabled.")
        else:
            self.game_path_entry.setText(game_path)
            self.append_to_console("Auto detection enabled.")

    
    def critical_messagebox(self, title, message):
        msg_box = QMessageBox(self)
        msg_box.setIcon(QMessageBox.Critical)
        msg_box.setWindowTitle(f"{title}")
        msg_box.setText(f"{message}")
        msg_box.setStyleSheet("""
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
        """)
        msg_box.exec()

    def info_messagebox(self, title, message):
        msg_box = QMessageBox(self)
        msg_box.setIcon(QMessageBox.Information)
        msg_box.setWindowTitle(f"{title}")
        msg_box.setText(f"{message}")
        msg_box.setStyleSheet("""
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
    """)
        msg_box.exec()

    def apply_styles(self):
        """Applies styles to application"""
        self.setStyleSheet("""
            QWidget {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                color: #fff;
            }
            QLabel, QPush_button, QCheckBox {
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
        """)
        # QCheckBox::indicator:checked {
        #         image: url(checked_checkbox.png); /* You need to provide this */
        #     }
        # QCheckBox::indicator:unchecked {
        #         image: url(unchecked_checkbox.png); /* You need to provide this */
        #     }
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
