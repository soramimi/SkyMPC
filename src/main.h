#ifndef MAIN_H
#define MAIN_H

#include <QString>

extern char const *SkyMPC_Version;
extern int const SkyMPC_Year;

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

extern bool start_with_shift_key;
extern QString application_data_dir;

#endif // MAIN_H
