
/* clang produces a slightly smaller binary compared to gcc */
/* it's used as an alternative to gcc */
`sudo apt install clang`


/* in most linux we have `gcc` and `make` installed built-in but make sure they are installed. */
`sudo apt install gcc make`

/* installing mingw for gcc cross-compiler installation to create PE files and support UEFI programming. the package name might look different. install the most similler name. */
`sudo apt install gcc-mingw-w64-x86-64`
or
`sudo apt search gcc-mingw-w64`

/* installing qemu for quick emulator */
`sudo apt install qemu-system-x86`

/* install ovmf (Open Virtual Machine Firmware), UEFI firmware for virtual machine. */
`sudo apt install ovmf`

/* uefi gpt image creator */
`git clone https://github.com/queso-fuego/UEFI-GPT-image-creator.git`

/* start the qemu virtual machine with ovmf */
`qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -net none`
`qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -net none`

/* to write on a usb drive or flash drive */
`dd -if=disk.img of=/dev/sdb00usb BS=1M`

/* cmpile the programme with gcc */
`gcc efi.c -std=c17 -Wall -Wextra -Wpendantic -mno-red-zone -ffreestanding -nostdlib -Wl, --subsystem,10 -e efi_main -o BOOTX64.EFI`

/* compile with the help of clang */
`clang efi.c -target x86_64-unknown-windows -std=c17 -Wall -Wextra -Wpendantic -mno-red-zone -ffreestanding -nostdlib -fuse-ld=lld-link  -Wl,-subsystem:efi_application -Wl,-entry:efi_main -o BOOTX64.EFI`

/* copy the bootx64.efi file into the gpt_creator folder. execute write_gpt. it will create a test.img file */

/* run qemu with the .img file. we just created */
`qemu-system-x86_64 -drive format=raw,unit=0,file=test.img -bios bios64.bin -m 256M -display sdl -vga std -name TESTOS -machine q35 -net none`
`qemu-system-x86_64 -drive format=raw,unit=0,file=test.img -bios bios64.bin -m 256M sdl -vga std -name TESTOS -machine q35 -net none`



###########################################################################
            EDK II Aproach
#########################################################################


`sudo apt install build-essential uuid-dev iasl git gcc qemu-system-x86 ovmf python3-distutils`

`git clone https://github.com/tianocore/edk2.git`
`cd edk2`
`git submodule update --init --recursive`
