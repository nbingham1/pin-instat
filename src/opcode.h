#pragma once

#include <stdint.h>

extern "C" {
#include <xed-interface.h>
}

struct opcode_t {
	opcode_t();
	opcode_t(int category);
	~opcode_t();

	int category;
	uint64_t count;
	uint64_t mem_count;
	uint64_t int_bitwidth[65];
	uint64_t sig_lbitwidth[53];
	uint64_t sig_mbitwidth[53];
	uint64_t exp_bitwidth[17];
};

opcode_t update_opcode(opcode_t a, opcode_t b);

uint64_t get_ufield(uint8_t *val, uint32_t start, uint32_t length);
int64_t get_sfield(uint8_t *val, uint32_t start, uint32_t length);
int get_int_bitwidth(int64_t val);
int get_uint_bitwidth(uint64_t val);
int get_msbf_bitwidth(uint64_t val, uint32_t width);
int get_sig_lbitwidth(uint8_t *val, uint32_t type);
int get_sig_mbitwidth(uint8_t *val, uint32_t type);
int get_exp_bitwidth(uint8_t *val, uint32_t type);
uintptr_t memory_getvalue(uintptr_t addr, uintptr_t size);

enum opcode_category
{
	ARITH_INT = 0,
	ARITH_REAL = 1,
	ROUTE_INT = 2,
	ROUTE_REAL = 3,
	CTRL_JUMP = 4,
	CTRL_OTHER = 5
};

int get_category(uint32_t opcode);

