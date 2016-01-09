#include "Toast.h"

Toast::Toast(QWidget *parent, QString const &text, Length length)
	: QLabel(parent)
{
	setWindowFlags(Qt::ToolTip | Qt::CustomizeWindowHint);
	setFocusPolicy(Qt::NoFocus);
	setAlignment(Qt::AlignCenter);
	setText(text);
	delay = (int)length;

	QString style = " *{";
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

void Toast::onTimeout()
{
	delete this;
}

void Toast::show(QWidget *parent, QString const &text, Length length)
{
	Toast *p = new Toast(parent, text, length);
	p->QLabel::show();
}
