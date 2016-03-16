
#ifndef __PATHCAT_H
#define __PATHCAT_H

#include <string>

std::string pathcat(char const *left, char const *right);
std::string pathcat(std::string const &left, std::string const &right);

#include <QString>
QString qpathcat(ushort const *left, ushort const *right);
inline QString pathcat(QString const &left, QString const &right)
{
	return qpathcat(left.utf16(), right.utf16());
}

static inline QString operator / (QString const &left, QString const &right)
{
	return pathcat(left, right);
}

#endif
