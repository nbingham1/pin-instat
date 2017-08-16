#pragma once

#include <pin.H>

struct opcode_t {
	opcode_t();
	opcode_t(int category);
	~opcode_t();

	int category;
	UINT64 count;
	UINT64 mem_count;
	UINT64 int_bitwidth[65];
	UINT64 sig_lbitwidth[53];
	UINT64 sig_mbitwidth[53];
	UINT64 exp_bitwidth[17];
};

opcode_t update_opcode(opcode_t a, opcode_t b);

UINT64 get_ufield(UINT8 *val, UINT32 start, UINT32 length);
INT64 get_sfield(UINT8 *val, UINT32 start, UINT32 length);
int get_int_bitwidth(INT64 val);
int get_uint_bitwidth(UINT64 val);
int get_msbf_bitwidth(UINT64 val, UINT32 width);
int get_sig_lbitwidth(UINT8 *val, UINT32 type);
int get_sig_mbitwidth(UINT8 *val, UINT32 type);
int get_exp_bitwidth(UINT8 *val, UINT32 type);
ADDRINT memory_getvalue(ADDRINT addr, ADDRINT size);

enum opcode_category
{
	ARITH_INT = 0,
	ARITH_REAL = 1,
	ROUTE_INT = 2,
	ROUTE_REAL = 3,
	CTRL_JUMP = 4,
	CTRL_OTHER = 5
};

int get_category(UINT32 opcode);

