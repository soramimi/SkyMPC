#include "Toast.h"
#include "BasicMainWindow.h"

Toast::Toast(BasicMainWindow *parent, QString const &text, Length length)
	: QLabel(parent)
{
	setWindowFlags(Qt::ToolTip | Qt::CustomizeWindowHint);
	setFocusPolicy(Qt::NoFocus);
	setAlignment(Qt::AlignCenter);
	setText(text);
	delay = (int)length;

	QString style = "*{";
	style += "padding: 10px;";
	style += "background-color: #606060;";
	style += "border: 1px solid #a0a0a0;";
	style += "color: #ffffff;";
	style += "}";
	setStyleSheet(style);

	connect(&timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	timer.setSingleShot(true);
	timer.start(delay);
}

Toast::~Toast()
{
}

void Toast::onTimeout()
{
	disconnect(&timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	setParent(0);
	this->deleteLater();
//	delete this;
}

void Toast::showEvent(QShowEvent *e)
{
	QLabel::showEvent(e);

	QWidget *widget = dynamic_cast<QWidget *>(parent());
	Q_ASSERT(widget);
	QRect r = widget->geometry();
	int x = r.x() + r.width() / 2;
	int y = r.y() + r.height() / 2;
	int w = width();
	int h = height();
	setGeometry(QRect(x - w / 2, y - h / 2, w, h));
}

void Toast::show(BasicMainWindow *parent, QString const &text, Length length)
{
	Toast *p = new Toast(parent, text, length);
	p->QLabel::show();
}
