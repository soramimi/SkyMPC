#-------------------------------------------------
#
# Project created by QtCreator 2013-06-22T12:02:00
#
#-------------------------------------------------

QT       += core gui xml network svg widgets

CONFIG += c++11

TARGET = SkyMPC
TEMPLATE = app

TRANSLATIONS = SkyMPC_ja.ts

SOURCES += src/main.cpp\
	src/MainWindow.cpp \
	src/MusicPlayerClient.cpp \
	src/MyTreeWidget.cpp \
	src/MySettings.cpp \
	src/MainWindowPrivate.cpp \
	src/MyListWidget.cpp \
	src/VerticalVolumePopup.cpp \
	src/AboutDialog.cpp \
	src/misc.cpp \
	src/SavePlaylistDialog.cpp \
	src/TestConnectResultDialog.cpp \
	src/pathcat.cpp \
	src/SongPropertyDialog.cpp \
	src/RenameDialog.cpp \
	src/AddLocationDialog.cpp \
	src/VolumeIndicatorPopup.cpp \
	src/VolumeIndicator.cpp \
	src/EditPlaylistDialog.cpp \
	src/NotifyOverlayWindow.cpp \
	src/KeyboardCustomizeDialog.cpp \
	src/KeyScanWidget.cpp \
	src/Server.cpp \
    src/ConnectionDialog.cpp \
    src/AskRemoveDuplicatedFileDialog.cpp \
    src/version.cpp \
    src/LegacyWindowsStyleTreeControl.cpp \
    src/win32.cpp \
	src/platform.cpp

HEADERS  += src/MainWindow.h \
	src/MusicPlayerClient.h \
	src/MyTreeWidget.h \
	src/MySettings.h \
	src/MainWindowPrivate.h \
	src/MyListWidget.h \
	src/main.h \
	src/VerticalVolumePopup.h \
	src/AboutDialog.h \
	src/misc.h \
	src/SavePlaylistDialog.h \
	src/EditPlaylistDialog.h \
	src/TestConnectResultDialog.h \
	src/pathcat.h \
	src/SongPropertyDialog.h \
	src/RenameDialog.h \
	src/AddLocationDialog.h \
	src/VolumeIndicatorPopup.h \
	src/VolumeIndicator.h \
	src/EditPlaylistDialog.h \
	src/NotifyOverlayWindow.h \
	src/KeyboardCustomizeDialog.h \
	src/KeyScanWidget.h \
	src/Server.h \
    src/ConnectionDialog.h \
    src/AskRemoveDuplicatedFileDialog.h \
    src/version.h \
    src/LegacyWindowsStyleTreeControl.h \
    src/win32.h \
	src/platform.h

FORMS    += src/MainWindow.ui \
	src/VerticalVolumePopup.ui \
	src/AboutDialog.ui \
	src/SavePlaylistDialog.ui \
	src/EditPlaylistDialog.ui \
	src/TestConnectResultDialog.ui \
	src/SongPropertyDialog.ui \
	src/RenameDialog.ui \
	src/AddLocationDialog.ui \
	src/VolumeIndicatorPopup.ui \
	src/EditPlaylistDialog.ui \
	src/NotifyOverlayWindow.ui \
	src/KeyboardCustomizeDialog.ui \
    src/ConnectionDialog.ui \
	src/AskRemoveDuplicatedFileDialog.ui

RESOURCES += \
	resources.qrc

win32 {
	LIBS += user32.lib
	LIBS += shell32.lib
	RC_FILE = SkyMPC.rc
	QMAKE_SUBSYSTEM_SUFFIX=,5.01
}

macx {
	QMAKE_INFO_PLIST = Info.plist
	ICON += SkyMPC.icns
	t.path=Contents/Resources
	t.files=SkyMPC_ja.qm
	QMAKE_BUNDLE_DATA += t
}

