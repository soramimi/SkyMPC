#ifndef APPLICATIONGLOBAL_H
#define APPLICATIONGLOBAL_H

#include <QString>


#define ORGANIZTION_NAME "soramimi.jp"
#define APPLICATION_NAME "SkyMPC"

struct ApplicationGlobal {
	bool start_with_shift_key = false;
	QString application_data_dir;
};

extern ApplicationGlobal *global;

#endif // APPLICATIONGLOBAL_H
