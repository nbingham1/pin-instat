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

int reg_id(REG reg) {
	switch (reg) {
	case REG_AL:
	case REG_AH:
	case REG_AX:
	case REG_EAX:
	case REG_RAX:
		return 0;
	case REG_BL:
	case REG_BH:
	case REG_BX:
	case REG_EBX:
	case REG_RBX:
		return 1;
	case REG_CL:
	case REG_CH:
	case REG_CX:
	case REG_ECX:
	case REG_RCX:
		return 2;
	case REG_DL:
	case REG_DH:
	case REG_DX:
	case REG_EDX:
	case REG_RDX:
		return 3;
	case REG_SIL:
	case REG_SI:
	case REG_ESI:
	case REG_RSI:
		return 4;
	case REG_DIL:
	case REG_DI:
	case REG_EDI:
	case REG_RDI:
		return 5;
	case REG_SPL:
	case REG_SP:
	case REG_ESP:
	case REG_RSP:
		return 6;
	case REG_BPL:
	case REG_BP:
	case REG_EBP:
	case REG_RBP:
		return 7;
	case REG_R8:
	case REG_R8B:
	case REG_R8W:
	case REG_R8D:
		return 8;
	case REG_R9:
	case REG_R9B:
	case REG_R9W:
	case REG_R9D:
		return 9;
	case REG_R10:
	case REG_R10B:
	case REG_R10W:
	case REG_R10D:
		return 10;
	case REG_R11:
	case REG_R11B:
	case REG_R11W:
	case REG_R11D:
		return 11;
	case REG_R12:
	case REG_R12B:
	case REG_R12W:
	case REG_R12D:
		return 12;
	case REG_R13:
	case REG_R13B:
	case REG_R13W:
	case REG_R13D:
		return 13;
	case REG_R14:
	case REG_R14B:
	case REG_R14W:
	case REG_R14D:
		return 14;
	case REG_R15:
	case REG_R15B:
	case REG_R15W:
	case REG_R15D:
		return 15;
	case REG_IP:
	case REG_EIP:
	case REG_RIP:
		return 16;
	case REG_SEG_CS:
		return 17;
	case REG_SEG_SS:
		return 18;
	case REG_SEG_DS:
		return 19;
	case REG_SEG_ES:
		return 20;
	case REG_SEG_FS:
		return 21;
	case REG_SEG_GS:
		return 22;
	case REG_CR0:
		return 23;
	case REG_CR1:
		return 24;
	case REG_CR2:
		return 25;
	case REG_CR3:
		return 26;
	case REG_CR4:
		return 27;
	case REG_DR0:
		return 28;
	case REG_DR1:
		return 29;
	case REG_DR2:
		return 30;
	case REG_DR3:
		return 31;
	case REG_DR4:
		return 32;
	case REG_DR5:
		return 33;
	case REG_DR6:
		return 34;
	case REG_DR7:
		return 35;
	case REG_K0:
		return 36;
	case REG_K1:
		return 37;
	case REG_K2:
		return 38;
	case REG_K3:
		return 39;
	 case REG_K4:
		return 40;
	case REG_K5:
		return 41;
	case REG_K6:
		return 42;
	case REG_K7:
		return 43;
	case REG_TSSR:
		return 44;
	case REG_LDTR:
		return 45;
	case REG_TR:
		return 46;
	case REG_TR3:
		return 47;
	case REG_TR4:
		return 48;
	case REG_TR5:
		return 49;
	case REG_TR6:
		return 50;
	case REG_TR7:
		return 51;
	case REG_MM0:
	case REG_ST0:
	case REG_EMM0:
	case REG_XMM0:
	case REG_YMM0:
	case REG_ZMM0:
		return 52;
	case REG_MM1:
	case REG_ST1:
	case REG_EMM1:
	case REG_XMM1:
	case REG_YMM1:
	case REG_ZMM1:
		return 53;
	case REG_MM2:
	case REG_ST2:
	case REG_EMM2:
	case REG_XMM2:
	case REG_YMM2:
	case REG_ZMM2:
		return 54;
	case REG_MM3:
	case REG_ST3:
	case REG_EMM3:
	case REG_XMM3:
	case REG_YMM3:
	case REG_ZMM3:
		return 55;
	case REG_MM4:
	case REG_ST4:
	case REG_EMM4:
	case REG_XMM4:
	case REG_YMM4:
	case REG_ZMM4:
		return 56;
	case REG_MM5:
	case REG_ST5:
	case REG_EMM5:
	case REG_XMM5:
	case REG_YMM5:
	case REG_ZMM5:
		return 57;
	case REG_MM6:
	case REG_ST6:
	case REG_EMM6:
	case REG_XMM6:
	case REG_YMM6:
	case REG_ZMM6:
		return 58;
	case REG_MM7:
	case REG_ST7:
	case REG_EMM7:
	case REG_XMM7:
	case REG_YMM7:
	case REG_ZMM7:
		return 59;
	case REG_XMM8:
	case REG_YMM8:
	case REG_ZMM8:
		return 60;
	case REG_XMM9:
	case REG_YMM9:
	case REG_ZMM9:
		return 61;
	case REG_XMM10:
	case REG_YMM10:
	case REG_ZMM10:
		return 62;
	case REG_XMM11:
	case REG_YMM11:
	case REG_ZMM11:
		return 63;
	case REG_XMM12:
	case REG_YMM12:
	case REG_ZMM12:
		return 64;
	case REG_XMM13:
	case REG_YMM13:
	case REG_ZMM13:
		return 65;
	case REG_XMM14:
	case REG_YMM14:
	case REG_ZMM14:
		return 66;
	case REG_XMM15:
	case REG_YMM15:
	case REG_ZMM15:
		return 67;
	case REG_XMM16:
	case REG_YMM16:
	case REG_ZMM16:
		return 68;
	case REG_XMM17:
	case REG_YMM17:
	case REG_ZMM17:
		return 69;
	case REG_XMM18:
	case REG_YMM18:
	case REG_ZMM18:
		return 70;
	case REG_XMM19:
	case REG_YMM19:
	case REG_ZMM19:
		return 71;
	case REG_XMM20:
	case REG_YMM20:
	case REG_ZMM20:
		return 72;
	case REG_XMM21:
	case REG_YMM21:
	case REG_ZMM21:
		return 73;
	case REG_XMM22:
	case REG_YMM22:
	case REG_ZMM22:
		return 74;
	case REG_XMM23:
	case REG_YMM23:
	case REG_ZMM23:
		return 75;
	case REG_XMM24:
	case REG_YMM24:
	case REG_ZMM24:
		return 76;
	case REG_XMM25:
	case REG_YMM25:
	case REG_ZMM25:
		return 77;
	case REG_XMM26:
	case REG_YMM26:
	case REG_ZMM26:
		return 78;
	case REG_XMM27:
	case REG_YMM27:
	case REG_ZMM27:
		return 79;
	case REG_XMM28:
	case REG_YMM28:
	case REG_ZMM28:
		return 80;
	case REG_XMM29:
	case REG_YMM29:
	case REG_ZMM29:
		return 81;
	case REG_XMM30:
	case REG_YMM30:
	case REG_ZMM30:
		return 82;
	case REG_XMM31:
	case REG_YMM31:
	case REG_ZMM31:
		return 83;
	default:
		return -1;
	}
}

int reg_width(REG reg) {
	return _regWidthToBitWidth[REG_Width(reg)];
}

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
// {instruction address: opcode}
cached_map<uint64_t, assembly_t, 1000, &update_assembly> image("image.tbl");

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

	UINT64 instr_addr = INS_Address(ins);
	xed_decoded_inst_t *xed = INS_XedDec(ins);

	image.update(instr_addr, assembly_t(opcode, INS_Disassemble(ins).c_str()));

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
		INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_CONTEXT, IARG_UINT32, opcode, IARG_UINT64, instr_addr, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	} else {
		INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_CONTEXT, IARG_UINT32, opcode, IARG_UINT64, instr_addr, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	}

	IARGLIST_Free(args);
}

static void on_ins (CONTEXT *ctx, UINT32 opcode, UINT64 instr_addr, UINT32 ops, ...)
{
	static bool first_ins = true;
	static array<int> write_list;
	static array<ADDRINT> mem_list;
	if ((instructions.total & 16383) == 0)
	{
		registers.finish(logfp);
		instructions.finish(logfp);
		opcodes.save(logfp);
		fprintf(logfp, "%lu Instructions %d/%d Opcodes\n", instructions.total, opcodes.cache.size(), opcodes.store.size());
		fflush(stdout);
	}

	if (first_ins)
	{
		image.save(logfp);
		first_ins = false;
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
