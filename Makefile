

gdbstub-xtensa-core: gdbstub_rsp.c gdbstub_sys.c gdbstub_sys.h
	gcc -o gdbstub-xtensa-core gdbstub_rsp.c gdbstub_sys.c -lelf

.PHONY: clean
clean:
	rm -f gdbstub-xtensa-core
