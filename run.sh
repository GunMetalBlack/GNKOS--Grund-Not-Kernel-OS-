#assemble boot.s file
as --32 boot.s -o boot.o

#compile kernel.c file
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

gcc -m32 -c utils.c -o utils.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

gcc -m32 -c char.c -o char.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra

#linking the kernel with kernel.o and boot.o files
ld -m elf_i386 -T linker.ld kernel.o utils.o char.o boot.o -o GNKOS.bin -nostdlib

#check GNKOS.bin file is x86 multiboot file or not
grub2-file --is-x86-multiboot GNKOS.bin

#building the iso file
mkdir -p isodir/boot/grub
cp GNKOS.bin isodir/boot/GNKOS.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub2-mkrescue -o GNKOS.iso isodir

#run it in qemu
qemu-system-x86_64 -cdrom GNKOS.iso