#include <stdio.h>
#include <windows.h>
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc = 0;
	WCHAR **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	LPVOID sc = VirtualAlloc(NULL, 0x10000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	HANDLE hFile = CreateFileW(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	DWORD r = 0;
	ReadFile(hFile, sc, 0x10000, &r, NULL);
	CloseHandle(hFile);

	((void (*)())sc)();
}
