#include <stdio.h>
#include <windows.h>
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc = 0;
	WCHAR **argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	HANDLE hFile = CreateFileW(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	DWORD dwSize = GetFileSize(hFile, NULL);
	dwSize = (dwSize + 0xfff) & ~0xfff;

	LPVOID sc = VirtualAlloc(NULL, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	ReadFile(hFile, sc, dwSize, &dwSize, NULL);
	CloseHandle(hFile);

	return ((int (*)())sc)();
}
