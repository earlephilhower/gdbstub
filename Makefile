

gdbstub-xtensa-core: gdbstub_rsp.c gdbstub_sys.c gdbstub_sys.h Makefile gdbstub.h
	gcc -g -Wall -Werror -DDEBUG=0 -o gdbstub-xtensa-core gdbstub_rsp.c gdbstub_sys.c -lelf

.PHONY: clean
clean:
	rm -f gdbstub-xtensa-core
