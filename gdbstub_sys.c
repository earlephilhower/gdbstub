/*
 * Copyright (C) 2016  Matt Borgerson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "gdbstub.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <libelf.h>

// Static ensures all fields are initted to 0, so no need to check later on
static struct dbg_state dbg_state;

#define RAMSTART 0x3FFE8000
#define RAMLEN   (0x14000 + 0x4000)

void add_mem_region(uint32_t base, uint32_t size, uint8_t *data)
{
	mem_region *mem = (mem_region*)malloc(sizeof(mem_region));
	mem->base = base;
	mem->size = size;
	mem->data = data;
	mem->next = NULL;
	if (!dbg_state.memory) {
		dbg_state.memory = mem;
	} else {
		mem_region *here = dbg_state.memory;
		while (here->next) {
			here = here->next;
		}
		here->next = mem;
	}
}

void dbg_sys_load(const char *fname)
{
	char buff[256];
	const char *regs = "---- begin regs ----";
	const char *mem = "---- begin core ----";

	// Always add the RAM, even if it's not loaded.  We can fill w/data later
	uint8_t *ram = (uint8_t*)malloc(RAMLEN);
	memset(ram, 0xec, RAMLEN);
	add_mem_region(RAMSTART, RAMLEN, ram);

	FILE *fp = fopen(fname, "r");
	while (fgets(buff, sizeof(buff), fp)) {
		if (!strncmp(buff, regs, strlen(regs))) {
			fscanf(fp, "%x", &dbg_state.regs.pc);
			fscanf(fp, "%x", &dbg_state.regs.ps);
			fscanf(fp, "%x", &dbg_state.regs.sar);
			fscanf(fp, "%*x"); // VPRI
			for (int i=0; i<16; i++) {
				fscanf(fp, "%x", &dbg_state.regs.a[i]); // A[0]..A[15]
			}
			fscanf(fp, "%x", &dbg_state.regs.litbase);
			fscanf(fp, "%x", &dbg_state.regs.sr176);
			fscanf(fp, "%*x"); // SR208
		} else if (!strncmp(buff, mem, strlen(mem))) {
			for (int i=0; i<RAMLEN; i++ ) {
				int t;
				fscanf(fp, "%02x", &t);
				ram[i] = t;
			}
		}
	}
	// Adjust to pull off the preserve_regs call
	dbg_state.regs.pc = *(uint32_t*)&ram[dbg_state.regs.a[1] + 16 + 12 - RAMSTART];
	dbg_state.regs.a[15] += 0x20;
}


void dbg_sys_load_elf(const char *fname)
{
	int fd = open(fname, O_RDONLY);
	elf_version(EV_CURRENT);
	Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
	Elf32_Ehdr *ehdr = elf32_getehdr(elf);
	Elf32_Phdr *phdr = elf32_getphdr(elf);
	for (int i=0; i<ehdr->e_phnum; i++) {
		if (phdr[i].p_vaddr) {
			uint8_t *mem = (uint8_t*)malloc(phdr[i].p_memsz);
			pread(fd, mem, phdr[i].p_memsz, phdr[i].p_offset);
			add_mem_region(phdr[i].p_vaddr, phdr[i].p_memsz, mem);
		}
	}
	close(fd);
}

/*
 * Write one character to the debugging stream.
 */
int dbg_sys_putchar(int ch)
{
	int ret = putchar(ch);
	fflush(stdout);
	return ret;
}

/*
 * Read one character from the debugging stream.
 */
int dbg_sys_getc(void)
{
	int ret = getchar() & 0xff;
	return ret;
}

mem_region *dbg_find_mem(address addr)
{
	mem_region *mem = dbg_state.memory;
	// Skip along until we find the region with this data
	while (mem && ((addr < mem->base) || (addr >= (mem->base + mem->size)))){
		mem = mem->next;
	}
	return mem;
}

/*
 * Read one byte from memory.
 */
int dbg_sys_mem_readb(address addr, char *val)
{
	mem_region *mem = dbg_find_mem(addr);
	if (!mem) {
		return -1;
	}
	*val = mem->data[addr - mem->base];
	return 0;
}

/*
 * Write one byte to memory.
 */
int dbg_sys_mem_writeb(address addr, char val)
{
	mem_region *mem = dbg_find_mem(addr);
	if (!mem) {
		return -1;
	}
	mem->data[addr - mem->base] = val;
	return 0;
}

/*
 * Continue program execution.
 */
int dbg_sys_continue(void)
{
	return -1;
}

/*
 * Single step the next instruction.
 */
int dbg_sys_step(void)
{
	return -1;
}


extern int dbg_main(struct dbg_state *state);

void usage()
{
	fprintf(stderr, "USAGE: gdbstub-xtensa-core --log <logfile.txt> --elf </path/to/sketch.ino.elf>\n");
	exit(1);
}

int main(int argc, char **argv)
{
	const char *elf;
	const char *log;
	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i], "--log")) {
			log = argv[++i];
		} else if (!strcmp(argv[i], "--elf")) {
			elf = argv[++i];
		} else {
			usage();
		}
	}
	if (!elf || !log) {
		usage();
	}
	dbg_sys_load(log);
	dbg_sys_load_elf(elf);
	dbg_main(&dbg_state);
}

