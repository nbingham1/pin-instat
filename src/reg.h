#pragma once

#include <pin.H>

#include "cached_table.h"

struct register_count_t
{
	register_count_t();
	~register_count_t();

	UINT64 count[7];
};

bool operator==(register_count_t r0, register_count_t r1);
bool operator!=(register_count_t r0, register_count_t r1);

struct register_t
{
	register_t();
	~register_t();

	UINT64 reads;
	UINT64 writes;
	UINT64 last_read;
	UINT64 last_instr;
};

struct register_record_t {
	register_record_t();
	register_record_t(const char *fanout, const char *age);
	~register_record_t();

	cached_table<register_count_t, 1000> register_fanout;
	cached_table<register_count_t, 1000> register_age;
	register_t regs[84];

	void finish(FILE *log = stdout);
	void read(int register_id);
	void write(int register_id, UINT64 instr_count, FILE *log = stdout);
};

int reg_id(REG reg);
int reg_class(int register_id);
int reg_width(REG reg);

