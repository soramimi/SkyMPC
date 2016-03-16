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

