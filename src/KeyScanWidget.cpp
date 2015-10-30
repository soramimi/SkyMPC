#include "KeyScanWidget.h"
#include <QKeyEvent>

KeyScanWidget::KeyScanWidget(QWidget *parent) :
	QLineEdit(parent)
{
}

void KeyScanWidget::keyPressEvent(QKeyEvent *e)
{
	int k = e->key();
	Qt::KeyboardModifiers m = e->modifiers();

	QString text[10];

	int n = 0;

	bool ctrl =  m & Qt::ControlModifier;
	bool alt =   m & Qt::AltModifier;
	bool shift = m & Qt::ShiftModifier;

	if (ctrl)  text[n++] = "Ctrl";
	if (alt)   text[n++] = "Alt";
	if (shift) text[n++] = "Shift";

	if (k >= Qt::Key_0 && k <= Qt::Key_9) {
		text[n++] = QChar(k);
	} else if (k >= Qt::Key_A && k <= Qt::Key_Z) {
		text[n++] = QChar(k);
	} else {
		switch (k) {
		case Qt::Key_F1: text[n++] = "F1"; break;
		case Qt::Key_F2: text[n++] = "F2"; break;
		case Qt::Key_F3: text[n++] = "F3"; break;
		case Qt::Key_F4: text[n++] = "F4"; break;
		case Qt::Key_F5: text[n++] = "F5"; break;
		case Qt::Key_F6: text[n++] = "F6"; break;
		case Qt::Key_F7: text[n++] = "F7"; break;
		case Qt::Key_F8: text[n++] = "F8"; break;
		}
	}

	QString t;
	for (int i = 0; i < n; i++) {
		if (!t.isEmpty()) {
			t += '+';
		}
		t += text[i];
	}

	setText(t);

}

