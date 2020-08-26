#ifndef MAIN_H
#define MAIN_H

#include <QString>

enum {
	ITEM_RowRole = Qt::UserRole,
	ITEM_IsRoot,
	ITEM_IsFolder,
	ITEM_IsFile,
	ITEM_IsPlaylist,
	ITEM_PosRole,
	ITEM_PathRole,
	ITEM_RangeRole,
	ITEM_SongIdRole,
};

struct ApplicationSettings {
	bool remember_and_restore_window_position = true;
	bool enable_high_dpi_scaling = false;
	static ApplicationSettings defaultSettings()
	{
		ApplicationSettings s;
		return s;
	}
};

#endif // MAIN_H
