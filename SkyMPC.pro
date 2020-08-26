#-------------------------------------------------
#
# Project created by QtCreator 2013-06-22T12:02:00
#
#-------------------------------------------------

QT       += core gui xml network svg widgets

TARGET = SkyMPC
TEMPLATE = app

CONFIG += c++11

DESTDIR = $$PWD/_bin

unix:QMAKE_CXXFLAGS += -Wall -Wextra -Werror=return-type -Werror=trigraphs -Wno-switch
unix:QMAKE_RPATHDIR += $ORIGIN

#linux:QTPLUGIN += ibusplatforminputcontextplugin

INCLUDEPATH += $$PWD/src

TRANSLATIONS = SkyMPC_ja.ts

# execute 'ruby prepare.rb' automatically

prepare.target = prepare
prepare.commands = cd $$PWD && ruby -W0 prepare.rb
QMAKE_EXTRA_TARGETS += prepare
PRE_TARGETDEPS += prepare


SOURCES += src/main.cpp \
	src/ColorSlider.cpp \
	src/MainWindow.cpp \
	src/MusicPlayerClient.cpp \
	src/MyTreeWidget.cpp \
	src/MySettings.cpp \
	src/MainWindowPrivate.cpp \
	src/MyListWidget.cpp \
	src/RingSlider.cpp \
	src/VerticalVolumePopup.cpp \
	src/AboutDialog.cpp \
	src/joinpath.cpp \
	src/misc.cpp \
	src/SavePlaylistDialog.cpp \
	src/TestConnectResultDialog.cpp \
	src/pathcat.cpp \
	src/SongPropertyDialog.cpp \
	src/RenameDialog.cpp \
	src/VolumeIndicatorPopup.cpp \
	src/VolumeIndicator.cpp \
	src/EditPlaylistDialog.cpp \
	src/KeyboardCustomizeDialog.cpp \
	src/KeyScanWidget.cpp \
	src/Server.cpp \
	src/ConnectionDialog.cpp \
	src/LegacyWindowsStyleTreeControl.cpp \
	src/win32.cpp \
	src/platform.cpp \
	src/TestConnectionThread.cpp \
	src/EditLocationDialog.cpp \
	src/Toast.cpp \
	src/StatusThread.cpp \
	src/PlaylistFile.cpp \
	src/SelectLocationDialog.cpp \
	src/webclient.cpp \
	src/MemoryReader.cpp \
	src/LocationLineEdit.cpp \
	src/SleepTimerDialog.cpp \
	src/TinyMainWindow.cpp \
	src/TinyMainWindowPrivate.cpp \
	src/BasicMainWindow.cpp \
	src/ServersComboBox.cpp \
	src/TinyConnectionDialog.cpp \
	src/BasicConnectionDialog.cpp \
	src/AskRemoveOverlappedFileDialog.cpp \
    version.c \
    src/StatusLabel.cpp \
    src/SettingsDialog.cpp \
    src/AbstractSettingForm.cpp \
    src/SettingGeneralForm.cpp \
    src/ApplicationGlobal.cpp

HEADERS  += src/MainWindow.h \
	src/ColorSlider.h \
	src/MusicPlayerClient.h \
	src/MyTreeWidget.h \
	src/MySettings.h \
	src/MainWindowPrivate.h \
	src/MyListWidget.h \
	src/RingSlider.h \
	src/joinpath.h \
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
	src/VolumeIndicatorPopup.h \
	src/VolumeIndicator.h \
	src/KeyboardCustomizeDialog.h \
	src/KeyScanWidget.h \
	src/Server.h \
	src/ConnectionDialog.h \
	src/LegacyWindowsStyleTreeControl.h \
	src/win32.h \
	src/platform.h \
	src/TestConnectionThread.h \
	src/EditLocationDialog.h \
	src/Toast.h \
	src/StatusThread.h \
	src/PlaylistFile.h \
	src/SelectLocationDialog.h \
	src/webclient.h \
	src/MemoryReader.h \
	src/LocationLineEdit.h \
	src/SleepTimerDialog.h \
	src/TinyMainWindow.h \
	src/TinyMainWindowPrivate.h \
	src/Common.h \
	src/BasicMainWindow.h \
	src/ServersComboBox.h \
	src/TinyConnectionDialog.h \
	src/BasicConnectionDialog.h \
	src/AskRemoveOverlappedFileDialog.h \
    src/StatusLabel.h \
    src/SettingsDialog.h \
    src/AbstractSettingForm.h \
    src/SettingGeneralForm.h \
    src/ApplicationGlobal.h

FORMS    += src/MainWindow.ui \
	src/VerticalVolumePopup.ui \
	src/AboutDialog.ui \
	src/SavePlaylistDialog.ui \
	src/EditPlaylistDialog.ui \
	src/TestConnectResultDialog.ui \
	src/SongPropertyDialog.ui \
	src/RenameDialog.ui \
	src/VolumeIndicatorPopup.ui \
	src/KeyboardCustomizeDialog.ui \
	src/ConnectionDialog.ui \
	src/EditLocationDialog.ui \
	src/SelectLocationDialog.ui \
	src/SleepTimerDialog.ui \
	src/TinyMainWindow.ui \
	src/TinyConnectionDialog.ui \
	src/AskRemoveOverlappedFileDialog.ui \
    src/SettingsDialog.ui \
    src/SettingGeneralForm.ui

RESOURCES += \
	resources.qrc

win32 {
	LIBS += user32.lib
	LIBS += shell32.lib
	RC_FILE = win.rc
	QMAKE_SUBSYSTEM_SUFFIX=,5.01
}

macx {
	QMAKE_INFO_PLIST = Info.plist
	ICON += SkyMPC.icns
	t.path=Contents/Resources
	t.files=SkyMPC_ja.qm
	QMAKE_BUNDLE_DATA += t
}

haiku {
	LIBS += -lnetwork
}

