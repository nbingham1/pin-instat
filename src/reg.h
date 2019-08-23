#pragma once

#include <db/cached_table.h>

struct register_count_t
{
	register_count_t();
	~register_count_t();

	uint64_t count[7];
};

bool operator==(register_count_t r0, register_count_t r1);
bool operator!=(register_count_t r0, register_count_t r1);

struct reg_t
{
	reg_t();
	~reg_t();

	uint64_t reads;
	uint64_t writes;
	uint64_t last_read;
	uint64_t last_instr;
};

struct register_record_t {
	register_record_t();
	register_record_t(const char *fanout, const char *age);
	~register_record_t();

	core::cached_table<register_count_t, 1000> register_fanout;
	core::cached_table<register_count_t, 1000> register_age;
	reg_t regs[84];

	void finish(FILE *log = stdout);
	void read(int register_id);
	void write(int register_id, uint64_t instr_count, FILE *log = stdout);
};

int reg_class(int register_id);

