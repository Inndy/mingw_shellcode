all: main32.exe main64.exe execute_shellcode32.exe execute_shellcode64.exe

main64.S: main.c
	x86_64-w64-mingw32-gcc $^ -S -o $@ -O3
	sed \
		-e '/^\s\+\.seh_.*/d' \
		-e '/^\s\+\.p2align.*/d'  \
		-e '/^\s\+\.ident.*GCC.*/d' \
		-e '/^\s\+\.section/d' \
		$@ -i

main64.obj: main64.S
	x86_64-w64-mingw32-as $^ -o $@

main64.exe: main64.obj
	x86_64-w64-mingw32-ld $^ -nostdlib -estart -o $@

main64.bin: main64.obj
	x86_64-w64-mingw32-objcopy -O binary $^ $@

main32.S: main.c
	i686-w64-mingw32-gcc $^ -S -o $@ -O3
	sed \
		-e '1i.byte 0xe8,0,0,0,0\njmp _start\n' \
		-e '/^\s\+\.seh_.*/d' \
		-e '/^\s\+\.p2align.*/d'  \
		-e '/^\s\+\.ident.*GCC.*/d' \
		-e '/^\s\+\.cfi_/d' \
		-e '/^\s\+\.section/d' \
		$@ -i

main32.obj: main32.S
	i686-w64-mingw32-as $^ -o $@

main32.exe: main32.obj
	i686-w64-mingw32-ld $^ -nostdlib -e_start -o $@

main32_text.bin: main32.obj
	i686-w64-mingw32-objcopy -O binary -j.text $^ $@

main32_reloc.bin: main32.exe
	i686-w64-mingw32-objcopy -O binary -j.reloc $^ $@

main32.bin: main32_text.bin main32_reloc.bin
	cat $^ > $@

execute_shellcode64.exe: execute_shellcode.c
	x86_64-w64-mingw32-gcc execute_shellcode.c -eWinMain -nostdlib -lkernel32 -lmsvcrt -lshell32 -o execute_shellcode64.exe

execute_shellcode32.exe: execute_shellcode.c
	i686-w64-mingw32-gcc execute_shellcode.c -e_WinMain -nostdlib -lkernel32 -lmsvcrt -lshell32 -o execute_shellcode32.exe
