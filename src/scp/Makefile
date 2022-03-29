default: help

help:
	@(echo "" ;\
	  echo "--- available targets ---" ;\
	  echo "qemu  - build and run on Qemu" ;\
	  echo "clean - remove generated files" ;\
	  echo "")

scp.elf: scp.o
	or1k-elf-ld -nostdlib -z max-page-size=0x10 -Tscp.ld $< -o $@

scp.o: scp.s
	or1k-elf-as -Iqemu $< -o $@

scp.o: qemu/app.f qemu/uart.inc

scp.elf : Makefile scp.ld

.PHONY: qemu

qemu: scp.elf
	qemu-system-or1k -serial mon:stdio -kernel scp.elf -nographic

clean:
	rm -f *.o *.elf *.bin
