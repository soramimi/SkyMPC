#ifndef VOLUMEINDICATORPOPUP_H
#define VOLUMEINDICATORPOPUP_H

#include <QWidget>

namespace Ui {
class VolumeIndicatorPopup;
}

class VolumeIndicatorPopup : public QWidget
{
	Q_OBJECT

public:
	explicit VolumeIndicatorPopup(QWidget *parent = 0);
	~VolumeIndicatorPopup();

	void setValue(int v);

protected:
	void changeEvent(QEvent *e);

private:
	Ui::VolumeIndicatorPopup *ui;
};

#endif // VOLUMEINDICATORPOPUP_H
