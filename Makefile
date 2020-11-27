all: main32.exe main64.exe execute_shellcode32.exe execute_shellcode64.exe

main64.exe: main.c
	x86_64-w64-mingw32-gcc main.c -S -o main64.S -O3
	sed -e 's/^\s\+\.seh_.*//g' -e 's/^\s\+\.p2align.*//g'  -e 's/^\s\+\.ident.*GCC.*//g' main64.S -i
	x86_64-w64-mingw32-as main64.S -o main64.obj
	x86_64-w64-mingw32-ld main64.obj -nostdlib -estart -o main64.exe
	x86_64-w64-mingw32-objcopy -O binary main64.obj main64.bin
	x86_64-w64-mingw32-strip main64.exe

main32.exe: main.c
	i686-w64-mingw32-gcc main.c -S -o main32.S -O3
	sed -e 's/^\s\+\.seh_.*//g' -e 's/^\s\+\.p2align.*//g'  -e 's/^\s\+\.ident.*GCC.*//g' main32.S -i
	i686-w64-mingw32-as main32.S -o main32.obj
	i686-w64-mingw32-ld main32.obj -nostdlib -e_start -o main32.exe
	i686-w64-mingw32-objcopy -O binary main32.obj main32.bin
	i686-w64-mingw32-strip main32.exe

execute_shellcode64.exe:
	x86_64-w64-mingw32-gcc execute_shellcode.c -eWinMain -nostdlib -lkernel32 -lmsvcrt -lshell32 -o execute_shellcode64.exe

execute_shellcode32.exe:
	i686-w64-mingw32-gcc execute_shellcode.c -e_WinMain -nostdlib -lkernel32 -lmsvcrt -lshell32 -o execute_shellcode32.exe
