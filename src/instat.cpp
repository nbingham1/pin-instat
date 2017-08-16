#include <stdio.h>
#include <pin.H>
#include <map>
#include <string>
#include <algorithm>
#include <locale>
#include <csignal>

#include "table.h"
#include "cached_map.h"

#include "array.h"
#include "math.h"

#include "reg.h"
#include "opcode.h"
#include "instr.h"

extern "C" {
#include <xed-interface.h>
}

#ifdef _MSC_VER
# define PxPTR "%Ix"
#else
# ifdef TARGET_IA32
#  define PxPTR "%x"
# else
#  define PxPTR "%lx"
# endif
#endif

enum
{
	iarg_reg = 0,
	iarg_mem = 1,
	iarg_imm = 2
};

const char *logname = "pin.log";
FILE *logfp;

register_record_t registers("fanout.tbl", "age.tbl");
instruction_record_t instructions("memory.tbl", "instrs.tbl");
cached_map<uint64_t, opcode_t, 1000, &update_opcode> opcodes("opcodes.tbl");	// indexed by instruction opcode as obtained by INS_Opcode()

std::map<ADDRINT,string> symbols;
std::map<ADDRINT,std::pair<ADDRINT,string> > imgs;

static void img_load (IMG img, void *v);
static void img_unload (IMG img, void *v);
static void on_ins (CONTEXT *ctx, UINT32 opcode, ADDRINT instr_addr, UINT32 ops, ...);
static void instruction (INS ins, void *v);
static void on_finish (INT32 code, void *v);

static void img_load (IMG img, void *v) {
	fprintf(logfp, "load %s off=" PxPTR " low=" PxPTR " high=" PxPTR " start=" PxPTR " size=" PxPTR "\n",
			IMG_Name(img).c_str(),
			IMG_LoadOffset(img), IMG_LowAddress(img), IMG_HighAddress(img),
			IMG_StartAddress(img), IMG_SizeMapped(img));

	string name = IMG_Name(img);
	size_t start = name.find_last_of("/\\");
	start = start == string::npos ? 0 : start + 1;
	size_t end = name.find_first_of('.', start);
	end = end == string::npos ? name.length() : end;
	if (end > start)
		name = name.substr(start, end - start);
	imgs[IMG_HighAddress(img)] = std::pair<ADDRINT,string>(IMG_LowAddress(img), name);

	for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
		for(RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
			fprintf(logfp, PxPTR " %s\n", RTN_Address(rtn), RTN_Name(rtn).c_str());
			symbols[RTN_Address(rtn)] = RTN_Name(rtn);
		}
	}
}

static void img_unload (IMG img, void *v)
{
	fprintf(logfp, "unload %s off=" PxPTR " low=" PxPTR " high=" PxPTR " start=" PxPTR " size=" PxPTR "\n",
			IMG_Name(img).c_str(),
			IMG_LoadOffset(img), IMG_LowAddress(img), IMG_HighAddress(img),
			IMG_StartAddress(img), IMG_SizeMapped(img));
}

static void instruction (INS ins, void *v)
{
	UINT32 opcode = (UINT32)INS_Opcode(ins);
	if (!opcode)
		return;

	ADDRINT instr_addr = INS_Address(ins);
	xed_decoded_inst_t *xed = INS_XedDec(ins);

	opcode_t *operation = opcodes.get(opcode);
	operation->category = INS_Category(ins);

	IARGLIST args = IARGLIST_Alloc();

	UINT32 ops = 0;
	for (UINT32 i = 0; i < INS_OperandCount(ins); i++) {
		if (INS_OperandIsReg(ins, i)) {
			REG reg = INS_OperandReg(ins, i);
			int regid = reg_id(reg);

			if (regid >= 0) {
				BOOL read = INS_OperandRead(ins, i);
				BOOL write = INS_OperandWritten(ins, i);

				if (REG_is_br(reg)) {	// ignore branch displacement registers
				} else if (REG_is_seg(reg)) { // ignore instruction address registers
				} else if (REG_is_any_mask(reg)) { // ignore k-mask registers
				} else if (REG_is_mxcsr(reg)) { // ignore sse control and status registers
				} else if (REG_is_fpst(reg)) { // ignore floating point control and status registers
				} else if (REG_is_any_flags_type(reg)) { // ignore all flag registers
				} else {
					UINT32 dwidth, dcount, dtype;
					dwidth = xed_decoded_inst_operand_element_size_bits(xed, i);
					dcount = xed_decoded_inst_operand_elements(xed, i);
					dtype = xed_decoded_inst_operand_element_type(xed, i);

					IARGLIST_AddArguments(args, IARG_UINT32, iarg_reg,
						IARG_UINT32, regid,
						IARG_UINT32, (UINT32)read,
						IARG_UINT32, (UINT32)write,
						IARG_UINT32, reg,
						IARG_UINT32, dcount,
						IARG_UINT32, dwidth,
						IARG_UINT32, dtype,
						IARG_END);
					ops++;
				}
			}
		} else if (INS_OperandIsImmediate(ins, i)) {
			UINT32 dsigned = xed_decoded_inst_get_immediate_is_signed(xed);

			IARGLIST_AddArguments(args, IARG_UINT32, iarg_imm,
				IARG_ADDRINT, INS_OperandImmediate(ins, i),
				IARG_UINT32, dsigned,
				IARG_END);
			ops++;
		}
	}

	for (UINT32 i = 0; i < INS_MemoryOperandCount(ins); i++) {
		BOOL read = INS_MemoryOperandIsRead(ins, i);
		BOOL write = INS_MemoryOperandIsWritten(ins, i);
		IARGLIST_AddArguments(args, IARG_UINT32, iarg_mem,
			IARG_UINT32, (UINT32)read,
			IARG_UINT32, (UINT32)write,
			IARG_MEMORYOP_EA, i,
			IARG_MEMORYREAD_SIZE,
			IARG_END);
		ops++;
	}

	if (INS_IsPredicated(ins)) {
		INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_CONTEXT, IARG_UINT32, opcode, IARG_ADDRINT, instr_addr, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	} else {
		INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_CONTEXT, IARG_UINT32, opcode, IARG_ADDRINT, instr_addr, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	}

	IARGLIST_Free(args);
}

static void on_ins (CONTEXT *ctx, UINT32 opcode, ADDRINT instr_addr, UINT32 ops, ...)
{
	static array<int> write_list;
	static array<ADDRINT> mem_list;
	if ((instructions.total & 4095) == 0)
	{
		fprintf(logfp, "%lu Instructions %d/%d Opcodes\n", instructions.total, opcodes.cache.size(), opcodes.store.size());
		fflush(stdout);
	}

	static int int_bitwidth[64] = {
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1};
	static int sig_lbitwidth[32] = {
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1};
	static int sig_mbitwidth[32] = {
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1};
	static int exp_bitwidth[32] = {
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1};

	bool is_mem = false;

	static UINT8 rval[64];

	// Iterate through the source operands
	va_list lst;
	va_start(lst, ops);
	for (UINT32 i = 0; i < ops; i++) {
		UINT32 type = va_arg(lst, UINT32);
		UINT32 is_read, is_write;
		UINT32 dcount, dwidth, dtype;
		REG reg;
		int regid;
		UINT8 *loc;
		int off;
		ADDRINT imm;
		INT64 mem;
		UINT32 dsigned;
		ADDRINT mem_addr, mem_size;

		switch (type) {
		case iarg_reg:
			regid = va_arg(lst, INT32);
			is_read = va_arg(lst, UINT32);
			is_write = va_arg(lst, UINT32);
			reg = (REG)va_arg(lst, UINT32);
			dcount = va_arg(lst, UINT32);
			dwidth = va_arg(lst, UINT32);
			dtype = va_arg(lst, UINT32);

			if (is_read) {
				PIN_GetContextRegval(ctx, reg, rval);
				
				loc = rval;
				off = dwidth>>3;
				switch (dtype) {
				case XED_OPERAND_ELEMENT_TYPE_UINT:
					for (UINT32 j = 0; j < dcount; j++, loc += off) {
						int_bitwidth[j] = std::max((int)int_bitwidth[j], get_uint_bitwidth(*(UINT64*)loc));
					}
					break;
				case XED_OPERAND_ELEMENT_TYPE_INT:
					for (UINT32 j = 0; j < dcount; j++, loc += off) {
						int_bitwidth[j] = std::max(int_bitwidth[j], get_int_bitwidth(*(INT64*)loc)); 
					}
					break;
				default: // floating point
					for (UINT32 j = 0; j < dcount; j++, loc += off) {
						sig_lbitwidth[j] = std::max(sig_lbitwidth[j], get_sig_lbitwidth(loc, dtype));
						sig_mbitwidth[j] = std::max(sig_mbitwidth[j], get_sig_mbitwidth(loc, dtype));
						exp_bitwidth[j] = std::max(exp_bitwidth[j], get_exp_bitwidth(loc, dtype));
					}
					break;
				}

				if (regid >= 0) {
					registers.read(regid);
				}
			}
			
			if (is_write && regid >= 0) {
				write_list.push_back(regid);
			}
			break;
		case iarg_imm:
			imm = va_arg(lst, ADDRINT);
			dsigned = va_arg(lst, UINT32);

			if (dsigned) {
				int_bitwidth[0] = std::max(int_bitwidth[0], get_int_bitwidth((INT64)imm));
			} else {
				int_bitwidth[0] = std::max(int_bitwidth[0], get_uint_bitwidth((UINT64)imm));
			}
			break;
		case iarg_mem:
			is_read = va_arg(lst, UINT32);
			is_write = va_arg(lst, UINT32);
			mem_addr = va_arg(lst, ADDRINT);
			mem_size = va_arg(lst, ADDRINT);
			if (is_read) {
				instructions.read(instr_addr, mem_addr, logfp);
				mem = memory_getvalue(mem_addr, mem_size);
				int_bitwidth[0] = std::max(int_bitwidth[0], get_int_bitwidth(mem));
				is_mem = true;
			}
			
			if (is_write) {
				mem_list.push_back(mem_addr);
				is_mem = true;
			}
			break;
		}
	}
	va_end(lst);

	for (array<int>::iterator i = write_list.begin(); i != write_list.end(); i++)
		registers.write(*i, instructions.total, logfp);
	write_list.clear();

	for (array<ADDRINT>::iterator i = mem_list.begin(); i != mem_list.end(); i++)
		instructions.write(instr_addr, *i, logfp);
	mem_list.clear();

	opcode_t *operation = opcodes.get(opcode);
	for (UINT32 i = 0; i < 64 && int_bitwidth[i] >= 0; i++) {
		operation->int_bitwidth[int_bitwidth[i]]++;
		int_bitwidth[i] = -1;
	}
	
	for (UINT32 i = 0; i < 32 && sig_lbitwidth[i] >= 0; i++) {
		operation->sig_lbitwidth[sig_lbitwidth[i]]++;
		sig_lbitwidth[i] = -1;
	}

	for (UINT32 i = 0; i < 32 && sig_mbitwidth[i] >= 0; i++) {
		operation->sig_mbitwidth[sig_mbitwidth[i]]++;
		sig_mbitwidth[i] = -1;
	}

	for (UINT32 i = 0; i < 32 && exp_bitwidth[i] >= 0; i++) {
		operation->exp_bitwidth[exp_bitwidth[i]]++;
		exp_bitwidth[i] = -1;
	}

	operation->count++;
	if (is_mem)
		operation->mem_count++;
	
	instructions.exec(instr_addr, logfp);
}

static void on_finish (INT32 code, void *v)
{
	registers.finish(logfp);
	instructions.finish(logfp);
	opcodes.save(logfp);

	fprintf(logfp, "Exit %d\n", code);
	fclose(logfp);
}

bool handle_signal(THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo, VOID *v)
{	
	exit(sig);
	return true;
}

int main (int argc, char *argv[])
{
	if(PIN_Init(argc, argv)) {
		fprintf(stderr, "command line error\n");
		return 1;
	}

	logfp = fopen(logname, "w");

	PIN_InitSymbols();

	PIN_InterceptSignal(SIGTERM, handle_signal, NULL);
	PIN_AddFiniFunction(on_finish, 0);
	IMG_AddInstrumentFunction(img_load, NULL);
	IMG_AddUnloadFunction(img_unload, NULL);
	INS_AddInstrumentFunction(instruction, NULL);

	PIN_StartProgram(); // Never returns
	return 0;
}
