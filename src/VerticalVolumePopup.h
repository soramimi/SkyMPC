#ifndef VOLUMEPOPUP_H
#define VOLUMEPOPUP_H

#include <QDialog>

namespace Ui {
class VerticalVolumePopup;
}

class VerticalVolumePopup : public QDialog
{
	Q_OBJECT
	
public:
	explicit VerticalVolumePopup(QWidget *parent = 0);
	~VerticalVolumePopup();
	int value();
	void setValue(int n);
	virtual void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *e);
private slots:
	void on_spinBox_valueChanged(int arg1);

private:
	Ui::VerticalVolumePopup *ui;
signals:
	void valueChanged();
};

#endif // VOLUMEPOPUP_H
