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

void diagnostic()
{
	QString text = "SkyMPC Diagnostic\n";
	text += QString("Version: ") + SkyMPC_Version + '\n';
	text += QString("Qt(Compile): ") + QT_VERSION_STR + '\n';
	text += QString("Qt(Runtime): ") + qVersion() + '\n';

	std::vector<Module> modsvec;

	HMODULE hPSAPI = LoadLibraryA("psapi.dll");
	if (hPSAPI) {
		fn_EnumProcessModules_t enumprocmod;
		fn_GetModuleBaseNameA_t getmodbasename;
		fn_GetModuleInformation_t getmodinfo;
		(FARPROC &)enumprocmod = GetProcAddress(hPSAPI, "EnumProcessModules");
		(FARPROC &)getmodbasename = GetProcAddress(hPSAPI, "GetModuleBaseNameA");
		(FARPROC &)getmodinfo = GetProcAddress(hPSAPI, "GetModuleInformation");
		if (enumprocmod && getmodbasename && getmodinfo) {
			HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
			if (hProc) {
				DWORD bytes;
				if (enumprocmod(hProc, 0, 0, &bytes)) {
					std::vector<char> modsbuf(bytes);
					HMODULE *mods = (HMODULE *)&modsbuf[0];
					if (enumprocmod(hProc, mods, bytes, &bytes)) {
						int n = bytes / sizeof(HMODULE);
						for (int i = 0; i < n; i++) {
							Module m;
							char tmp[1000];
							MODULEINFO mi;
							getmodbasename(hProc, mods[i], tmp, sizeof(tmp));
							m.name = tmp;
							getmodinfo(hProc, mods[i], &mi, sizeof(MODULEINFO));
							m.base = (DWORD)mi.lpBaseOfDll;
							m.size = mi.SizeOfImage;
							modsvec.push_back(m);
						}
					}
				}
				CloseHandle(hProc);
			}
		}
		FreeLibrary(hPSAPI);
	}
	if (modsvec.empty()) {
		text += the_mainwindow->tr_Module_information_could_not_be_acquired() + '\n';
	} else {
		std::sort(modsvec.begin(), modsvec.end());
		for each (Module const &m in modsvec) {
			char tmp[100];
			sprintf(tmp, "%08X %08X ", m.base, m.size);
			text += tmp;
			text += QString::fromStdString(m.name);
			text += '\n';
		}
	}

	DiagnosticDialog dlg(the_mainwindow);
	dlg.setText(text);
	dlg.exec();
}


#endif
