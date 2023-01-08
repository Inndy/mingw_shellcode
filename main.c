#include <stdio.h>
#include <windows.h>
#include <winternl.h>

#define ENCODE_PTR(X) (((DWORD_PTR)(X)) - (DWORD_PTR)0xe8c7b756d76aa478)
#define DECODE_PTR(X) (LPVOID)(((DWORD_PTR)(X)) + (DWORD_PTR)0xe8c7b756d76aa478)
#define DECODE_INTPTR(X) (DWORD_PTR)DECODE_PTR(X)

#define STRCMP_INLINE(R, S1, S2) \
	do { \
		R = 0; \
		LPCSTR s1 = (LPCSTR)(S1), s2 = (LPCSTR)(S2); \
		do R |= (*s1 - *s2); while(*s1++ && *s2++); \
	} while(0)

#define STRCMPH_INLINE(R, PTR, HASH) \
	do { \
		LPCSTR s = (LPCSTR)(PTR); \
		R = 0; \
		DWORD h = 1; \
		SIZE_T i = 0; \
		BYTE k; \
		while(k = s[i++]) { \
			h = (h - k) * 0x314159; \
			h = h ^ (h >> 3); \
		} \
		h -= i; \
		R |= h ^ (DWORD)(HASH); \
	} while(0)

#define GET_PROC_INLINE(PTR, CMPR, BASE, FUNC) \
	do { \
		PIMAGE_NT_HEADERS pe_h = (PIMAGE_NT_HEADERS)(DECODE_INTPTR(BASE) + 0x40); \
		while(~*(DWORD*)pe_h != (DWORD)~0x4550) pe_h = (PIMAGE_NT_HEADERS)((DWORD_PTR)pe_h + 0x4); \
		PIMAGE_DATA_DIRECTORY data_directory = &pe_h->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]; \
		PIMAGE_EXPORT_DIRECTORY export_directory = (PIMAGE_EXPORT_DIRECTORY)(DECODE_INTPTR(BASE) + data_directory->VirtualAddress); \
		\
		DWORD *Functions = (DWORD *)(DECODE_INTPTR(BASE) + export_directory->AddressOfFunctions); \
		DWORD *Names = (DWORD *)(DECODE_INTPTR(BASE) + export_directory->AddressOfNames); \
		WORD *NameOrdinals = (WORD *)(DECODE_INTPTR(BASE) + export_directory->AddressOfNameOrdinals); \
		\
		DWORD i = -1, cmp; \
		do { CMPR(cmp, (LPCSTR)(DECODE_INTPTR(BASE) + Names[++i]), FUNC); } while(cmp); \
		\
		*(DWORD_PTR*)(&PTR) = DECODE_INTPTR(BASE) + Functions[NameOrdinals[i]]; \
		if((DWORD_PTR)export_directory <= *(DWORD_PTR*)(&PTR) && *(DWORD_PTR*)(&PTR) <= (DWORD_PTR)export_directory + data_directory->Size) \
			*(DWORD_PTR*)(&PTR) = 0; \
	} while(0)

#ifdef DEBUG
#define STRCMP(R, S1, S2) do { R = _strcmp(S1, S2); } while(0)
#define STRCMPH(R, PTR, HASH) do { R = _strcmph(PTR, HASH); } while(0)
#define GET_PROC_H(PTR, BASE, H) do { *(DWORD_PTR*)(&PTR) = _get_proc_h(BASE, H); } while(0)
#define GET_PROC(PTR, BASE, FNAME) do { *(DWORD_PTR*)(&PTR) = _get_proc(BASE, FNAME); } while(0)
#else
#define STRCMP STRCMP_INLINE
#define STRCMPH STRCMPH_INLINE
#define GET_PROC_H(PTR, BASE, H) GET_PROC_INLINE(PTR, STRCMPH, BASE, H)
#define GET_PROC(PTR, BASE, FNAME) GET_PROC_INLINE(PTR, STRCMP, BASE, FNAME)
#endif


#ifdef DEBUG
DWORD _strcmp(LPCSTR _s1, LPCSTR _s2)
{
	DWORD _r;
	STRCMP_INLINE(_r, _s1, _s2);
	return _r;
}

DWORD _strcmph(LPCSTR _s, DWORD _h)
{
	DWORD _r;
	STRCMPH_INLINE(_r, _s, _h);
	return _r;
}

DWORD_PTR _get_proc_h(DWORD_PTR base, DWORD h)
{
	DWORD_PTR p;
	GET_PROC_INLINE(p, STRCMPH, base, h);
	return p;
}

DWORD_PTR _get_proc(DWORD_PTR base, LPCSTR fname)
{
	DWORD_PTR p;
	GET_PROC_INLINE(p, STRCMP, base, fname);
	return p;
}
#endif


void start()
{
	DWORD_PTR kernel32;

	LIST_ENTRY *root = &((TEB*)NtCurrentTeb())->ProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList;
	LDR_DATA_TABLE_ENTRY *entry = CONTAINING_RECORD(root, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
	while(entry = CONTAINING_RECORD(entry->InMemoryOrderLinks.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks), &entry->InMemoryOrderLinks != root) {
		LPCWSTR dll_base_name = (1 + &entry->FullDllName)->Buffer;
		DWORD64 n = *(DWORD64*)dll_base_name | (*(DWORD64*)(dll_base_name + 4) << 8);
		if((0xdfdfdfdfdfdfdfdf & ~n) * ((1 + &entry->FullDllName)->Length ^ 0x26594131) == 0x126AF8D89A7655B4)
			kernel32 = ENCODE_PTR(entry->DllBase);
	}

	HMODULE WINAPI (*pLoadLibraryA)(LPCSTR);
	GET_PROC_H(pLoadLibraryA, kernel32, 0xeef37599);

	char str_dll_name[] = "user32.dll";
	DWORD_PTR user32 = ENCODE_PTR(pLoadLibraryA(str_dll_name));

	DWORD WINAPI (*pMessageBoxA)(HWND, LPCSTR, LPCSTR, DWORD);
	char str_messagebox_a[] = "MessageBoxA";
	GET_PROC(pMessageBoxA, user32, str_messagebox_a);

	CHAR text[] = "Hello, World!";
	CHAR title[] = "Hello";
	pMessageBoxA(NULL, text, title, MB_ICONINFORMATION | MB_SETFOREGROUND);

	DWORD WINAPI (*pExitProcess)(DWORD);
	char str_exit_process[] = "ExitProcess";
	GET_PROC(pExitProcess, kernel32, str_exit_process);
	pExitProcess(0);
}
