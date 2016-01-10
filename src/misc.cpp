#include "misc.h"
#include <QPainter>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>

void drawBox(QPainter *painter, int x, int y, int w, int h, QColor const &color)
{
	if (w > 0 && h > 0) {
		QBrush brush(color);
		if (w < 3 || h < 3) {
			painter->fillRect(x, y, w, h, brush);
		} else {
			painter->fillRect(x, y, w - 1, 1, brush);
			painter->fillRect(x, y + 1, 1, h - 1, brush);
			painter->fillRect(x + w - 1, y, 1, h - 1, brush);
			painter->fillRect(x + 1, y + h - 1, w - 1, 1, brush);
		}
	}
}

ushort *ucschr(ushort const *ptr, ushort c)
{
	while (*ptr) {
		if (*ptr == c) {
			return (ushort *)ptr;
		}
		ptr++;
	}
	return 0;
}

ushort *ucsrchr(ushort const *ptr, ushort c)
{
	ushort *r = 0;
	while (*ptr) {
		if (*ptr == c) {
			r = (ushort *)ptr;
		}
		ptr++;
	}
	return r;
}


QString removeKeyAcceleratorText(QString text)
{
	int dots = 0;
	int n = text.size();
	while (n > 0 && text[n - 1] == '.') {
		dots++;
		n--;
	}
	if (n > 4) {
		if (text[n - 1] == ')') {
			if (text[n - 2].isLetter()) {
				if (text[n - 3] == '&') {
					if (text[n - 4] == '(') {
						text = QString::fromUtf16(text.utf16(), n - 4);
						for (int i = 0; i < dots; i++) {
							text += '.';
						}
					}
				}
			}
		}
	}
	return text;
}


template <typename T> void remove_key_accelerator_test_(QObject *o)
{
	T *w = qobject_cast<T *>(o);
	if (w) {
		QString text = w->text();
		text = removeKeyAcceleratorText(text);
		w->setText(text);
	}
}

template <> void remove_key_accelerator_test_<QGroupBox>(QObject *o)
{
	QGroupBox *w = qobject_cast<QGroupBox *>(o);
	if (w) {
		QString text = w->title();
		text = removeKeyAcceleratorText(text);
		w->setTitle(text);
	}
}

void removeKeyAcceleratorText(QObject *obj)
{
	QObjectList list = obj->children();
    for (QObject *o : list) {
		removeKeyAcceleratorText(o);
		remove_key_accelerator_test_<QLabel>(o);
		remove_key_accelerator_test_<QGroupBox>(o);
		remove_key_accelerator_test_<QPushButton>(o);
		remove_key_accelerator_test_<QCheckBox>(o);
		remove_key_accelerator_test_<QRadioButton>(o);
	}
}

void pseudo_crypto_encode(char *ptr, int len)
{
	if (len > 1) {
		unsigned char *p = (unsigned char *)ptr;
		int n = len - 1;
		for (int i = 0; i < n; i++) {
			p[i + 1] = p[i + 1] ^ (p[i] * 27 + 13);
		}
		for (int i = 0; i < n; i++) {
			p[n - i - 1] = p[n - i - 1] ^ (p[n - i] * 31 + 11);
		}
	}
}

void pseudo_crypto_decode(char *ptr, int len)
{
	if (len > 1) {
		unsigned char *p = (unsigned char *)ptr;
		int n = len - 1;
		for (int i = 0; i < n; i++) {
			p[i] = p[i] ^ (p[i + 1] * 31 + 11);
		}
		for (int i = 0; i < n; i++) {
			p[n - i] = p[n - i] ^ (p[n - i - 1] * 27 + 13);
		}
	}
}

