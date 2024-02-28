QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    install_thread.cpp \
    main.cpp \
    utils.cpp

INCLUDEPATH += \
    include \
    C:/Users/Raiden/Documents/programming/Doki-Doki-Mod-Installer/zlib-1.3.1

LIBS += \
    -LC:/Users/Raiden/Documents/programming/Doki-Doki-Mod-Installer/zlib-1.3.1/build/Release -lzlibstatic

FORMS += \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    include/dimming_overlay.h \
    include/install_thread.h \
    include/mainwindow.h \
    include/signal_manager.h \
    include/utils.h
