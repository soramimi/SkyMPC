#include "LocationLineEdit.h"

#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

LocationLineEdit::LocationLineEdit(QWidget *parent)
	: QLineEdit(parent)
{

}

void LocationLineEdit::dropEvent(QDropEvent *e)
{
	QMimeData const *mimedata = e->mimeData();
	QList<QUrl> urls = mimedata->urls();
	if (urls.size() == 1) {
		setText(urls[0].url());
	}
}
