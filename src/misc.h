#ifndef MISC_H
#define MISC_H

#include <QtGlobal>

class QObject;
class QPainter;
class QColor;

void drawBox(QPainter *painter, int x, int y, int w, int h, QColor const &color);

ushort *ucschr(ushort const *ptr, ushort c);
ushort *ucsrchr(ushort const *ptr, ushort c);

void pseudo_crypto_encode(char *ptr, int len);
void pseudo_crypto_decode(char *ptr, int len);

#endif // MISC_H
