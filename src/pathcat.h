
#ifndef __PATHCAT_H
#define __PATHCAT_H

#include <string>

std::string pathcat(char const *left, char const *right);
std::wstring pathcat(wchar_t const *left, wchar_t const *right);
std::string pathcat(std::string const &left, std::string const &right);
std::wstring pathcat(std::wstring const &left, std::wstring const &right);

#ifdef QT_VERSION
QString qpathcat(ushort const *left, ushort const *right);
inline QString pathcat(QString const &left, QString const &right)
{
	return qpathcat(left.utf16(), right.utf16());
}
#endif

#endif
