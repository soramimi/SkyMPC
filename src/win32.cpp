#include <QtGlobal>
#ifdef Q_OS_WIN
#include "win32.h"
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <algorithm>
#include "version.h"
#include <QString>

#include "MainWindow.h"
#include "DiagnosticDialog.h"


struct Module {
	DWORD base;
	DWORD size;
	std::string name;
	bool operator < (Module const &r) const
	{
		return base < r.base;
	}
};

typedef BOOL (WINAPI *fn_EnumProcessModules_t)(HANDLE  hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef DWORD (WINAPI *fn_GetModuleBaseNameA_t)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
typedef BOOL (WINAPI *fn_GetModuleInformation_t)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);



#endif
