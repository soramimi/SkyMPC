#ifndef APPLICATIONGLOBAL_H
#define APPLICATIONGLOBAL_H

#include <QString>


#define ORGANIZATION_NAME "soramimi.jp"
#define APPLICATION_NAME "SkyMPC"

struct ApplicationGlobal {
	QString organization_name;
	QString application_name;
	QString generic_config_dir;
	QString app_config_dir;
	QString config_file_path;
	bool start_with_shift_key = false;
	QString application_data_dir;
};

extern ApplicationGlobal *global;

#endif // APPLICATIONGLOBAL_H
