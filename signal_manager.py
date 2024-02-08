from PySide6.QtCore import QObject, Signal
class SignalManager(QObject):
    console_update = Signal(str) 
    progress_update = Signal(float)
    critical_messagebox = Signal(str, str)

signal_manager = SignalManager()