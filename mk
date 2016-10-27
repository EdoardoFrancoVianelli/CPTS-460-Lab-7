VFD=mtximage

clear

as86 -o ts.o ts.s
bcc  -c -ansi t.c
ld86 -d -o mtx ts.o t.o OBJ/*.o mtxlib /usr/lib/bcc/libc.a

mount -o loop $VFD /mnt
cp mtx /mnt/boot
umount /mnt


(cd USER; ./mkallu)

qemu-system-x86_64 -fda mtximage -no-fd-bootchk

echo done
