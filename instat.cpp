#include <stdio.h>
#include <pin.H>
#include <map>
#include <string>
#include <algorithm>
#include <locale>

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

UINT64 ins_count = 0;

struct oprecord {
	oprecord() {
		count = 0;
		mem_count = 0;
		for (int i = 0; i < 65; i++)
			int_bitwidth[i] = 0;
		for (int i = 0; i < 53; i++) {
			sig_lbitwidth[i] = 0;
			sig_mbitwidth[i] = 0;
		}
		for (int i = 0; i < 17; i++)
			exp_bitwidth[i] = 0;
	}

	~oprecord() {}

	string category;
	UINT64 int_bitwidth[65];
	UINT64 sig_lbitwidth[53];
	UINT64 sig_mbitwidth[53];
	UINT64 exp_bitwidth[17];
	UINT64 count;
	UINT64 mem_count;
};

struct insrecord {
	insrecord() {
		count = 0;
	}

	UINT64 count;
	map<UINT64, UINT64> read_count;

	void read(UINT64 addr) {
		map<UINT64, UINT64>::iterator i = read_count.find(addr);
		if (i == read_count.end())
			read_count.insert(pair<UINT64, UINT64>(addr, 1));
		else
			i->second++;
	}
};

struct memtype {
	memtype() {}
	~memtype() {}

	map<UINT64, UINT64> fanout_distribution;
	map<UINT64, UINT64> age_distribution;

	void write(int fanout, int age) {
		map<UINT64, UINT64>::iterator j;

		j = fanout_distribution.find(fanout);
		if (j == fanout_distribution.end())
			fanout_distribution.insert(pair<UINT64, UINT64>(fanout, 1));
		else
			j->second++;

		j = age_distribution.find(age);
		if (j == age_distribution.end())
			age_distribution.insert(pair<UINT64, UINT64>(age, 1));
		else
			j->second++;
	}
};

struct memrecord {
	memrecord() {
		type = NULL;
		read_count = 0;
		write_count = 0;
		last_write_rd = 0;
		last_write_ins = 0;
	}

	~memrecord() {}

	memtype *type;
	UINT64 read_count;
	UINT64 write_count;
	UINT64 last_write_rd;
	UINT64 last_write_ins;

	void read() {
		read_count++;
	}

	void write() {
		int fanout = read_count - last_write_rd;
		int age = ins_count - last_write_ins;

		if (type)
			type->write(fanout, age);
		
		write_count++;
		last_write_rd = read_count;
		last_write_ins = ins_count;
	}
};

enum
{
	iarg_reg = 0,
	iarg_mem = 1,
	iarg_imm = 2
};

const char *logname = "pin.log";
const char *opname = "opcode.tsv";
const char *insname = "instruction.tsv";
const char *fanoutname = "fanout.tsv";
const char *agename = "age.tsv";
FILE *logfp;

memrecord regs[84];
memtype reg_types[7];
std::map<ADDRINT,insrecord> insmap;
std::map<string,oprecord> opmap;

std::map<ADDRINT,string> symbols;
std::map<ADDRINT,std::pair<ADDRINT,string> > imgs;

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

int reg_type(int id) {
	if (id < 0)
		return -1;
	else if (id < 16)
		return 0;
	else if (id < 23)
		return 1;
	else if (id < 28)
		return 2;
	else if (id < 36)
		return 3;
	else if (id < 44)
		return 4;
	else if (id < 52)
		return 5;
	else if (id < 84)
		return 6;
	else
		return -1;
}

int reg_width(REG reg) {
	return _regWidthToBitWidth[REG_Width(reg)];
}

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

UINT64 get_ufield(UINT8 *val, UINT32 start, UINT32 length)
{
	UINT64 sect = *(UINT64*)(val + (start>>3));
	UINT32 off = start & 7;
	sect <<= 64 - length - off;
	sect >>= 64 - length;
	return sect;
}

INT64 get_sfield(UINT8 *val, UINT32 start, UINT32 length)
{
	INT64 sect = *(INT64*)(val + (start>>3));
	UINT32 off = start & 7;
	sect <<= 64 - length - off;
	sect >>= 64 - length;
	return sect;
}

int get_int_bitwidth(INT64 val) {
	int bitwidth = 0;	
	INT64 test = val;
	while (test != 0 && test != -1) {
		test >>= 1;
		bitwidth++;
	}
	return bitwidth;
}

int get_uint_bitwidth(UINT64 val) {
	int bitwidth = 0;	
	UINT64 test = val;
	while (test != 0) {
		test >>= 1;
		bitwidth++;
	}
	return bitwidth;
}

int get_msbf_bitwidth(UINT64 val, UINT32 width) {
	int bitwidth = 0;	
	UINT64 test = val << (64-width);
	while (test != 0) {
		test <<= 1;
		bitwidth++;
	}
	return bitwidth;
}

int get_sig_lbitwidth(UINT8 *val, UINT32 type) {
	UINT32 sign;
	UINT32 width;
	switch (type) {
	case XED_OPERAND_ELEMENT_TYPE_FLOAT16:
		width = 10;
		sign = get_ufield(val, 15, 1);
		break;
	case XED_OPERAND_ELEMENT_TYPE_SINGLE:
		width = 23;
		sign = get_ufield(val, 31, 1);
		break;
	case XED_OPERAND_ELEMENT_TYPE_DOUBLE:
		width = 52;
		sign = get_ufield(val, 63, 1);
		break;
	case XED_OPERAND_ELEMENT_TYPE_LONGDOUBLE:
		width = 64;
		sign = get_ufield(val, 79, 1);
		break;
	default:
		return -1;
	}

	INT64 field = get_sfield(val, 0, width);
	if (sign)
		field = -field;	

	return get_int_bitwidth(field);
}

int get_sig_mbitwidth(UINT8 *val, UINT32 type) {
	UINT32 sign;
	UINT32 width;
	switch (type) {
	case XED_OPERAND_ELEMENT_TYPE_FLOAT16:
		width = 10;
		sign = get_ufield(val, 15, 1);
		break;
	case XED_OPERAND_ELEMENT_TYPE_SINGLE:
		width = 23;
		sign = get_ufield(val, 31, 1);
		break;
	case XED_OPERAND_ELEMENT_TYPE_DOUBLE:
		width = 52;
		sign = get_ufield(val, 63, 1);
		break;
	case XED_OPERAND_ELEMENT_TYPE_LONGDOUBLE:
		width = 64;
		sign = get_ufield(val, 79, 1);
		break;
	default:
		return -1;
	}

	INT64 field = get_sfield(val, 0, width);
	if (sign)
		field = -field;

	return get_msbf_bitwidth(field, width);
}


int get_exp_bitwidth(UINT8 *val, UINT32 type) {
	INT64 field = 0;
	switch (type) {
	case XED_OPERAND_ELEMENT_TYPE_FLOAT16:
		field = get_sfield(val, 10, 5) - 15;
		break;
	case XED_OPERAND_ELEMENT_TYPE_SINGLE:
		field = get_sfield(val, 23, 8) - 127;
		break;
	case XED_OPERAND_ELEMENT_TYPE_DOUBLE:
		field = get_sfield(val, 52, 11) - 1023;
		break;
	case XED_OPERAND_ELEMENT_TYPE_LONGDOUBLE:
		field = get_sfield(val, 64, 15) - 16383;
		break;
	default:
		return -1;
	}

	return get_int_bitwidth(field);
}

ADDRINT memory_getvalue(ADDRINT addr, ADDRINT size)
{
	ADDRINT val;
	switch (size) {
		case 1: val = *(UINT8 *)addr; break;
		case 2: val = *(UINT16 *)addr; break;
		case 4: val = *(UINT32 *)addr; break;
#ifdef TARGET_IA32E
		case 8: val = *(UINT64 *)addr; break;
#endif
		default: val = *(ADDRINT *)addr;
	}
	return val;
}

static void on_ins (CONTEXT *ctx, oprecord *opcode, insrecord *instr, UINT32 ops, ...)
{
	va_list lst;
	int int_bitwidth[64];
	int sig_lbitwidth[32];
	int sig_mbitwidth[32];
	int exp_bitwidth[32];
	for (int i = 0; i < 32; i++)
	{
		int_bitwidth[i] = -1;
		int_bitwidth[i+32] = -1;
		sig_lbitwidth[i] = -1;
		sig_mbitwidth[i] = -1;
		exp_bitwidth[i] = -1;
	}

	bool is_mem = false;
	UINT32 max_count = 1;

	UINT8 rval[64];

	// Iterate through the source operands
	va_start(lst, ops);
	for (UINT32 i = 0; i < ops; i++) {
		UINT32 type = va_arg(lst, UINT32);
		if (type == iarg_reg) {
			int regid = va_arg(lst, INT32);
			UINT32 is_read = va_arg(lst, UINT32);
			va_arg(lst, UINT32);
			REG reg = (REG)va_arg(lst, UINT32);
			UINT32 dcount = va_arg(lst, UINT32);
			UINT32 dwidth = va_arg(lst, UINT32);
			UINT32 dtype = va_arg(lst, UINT32);

			if (is_read) {
				PIN_GetContextRegval(ctx, reg, rval);
				if (dcount > max_count)
					max_count = dcount;

				for (UINT32 j = 0; j < dcount; j++) {
					if (dtype == XED_OPERAND_ELEMENT_TYPE_UINT) {
						UINT64 val = *(UINT64*)(rval + j*(dwidth>>3));
						int_bitwidth[j] = max(int_bitwidth[j], get_uint_bitwidth(val));
					} else if (dtype == XED_OPERAND_ELEMENT_TYPE_INT) {
						INT64 val = *(INT64*)(rval + j*(dwidth>>3));
						int_bitwidth[j] = max(int_bitwidth[j], get_int_bitwidth(val)); 
					} else { // floating point
						sig_lbitwidth[j] = max(sig_lbitwidth[j], get_sig_lbitwidth(rval + j*(dwidth>>3), dtype));
						sig_mbitwidth[j] = max(sig_mbitwidth[j], get_sig_mbitwidth(rval + j*(dwidth>>3), dtype));
						exp_bitwidth[j] = max(exp_bitwidth[j], get_exp_bitwidth(rval + j*(dwidth>>3), dtype));
					}
				}

				if (regid >= 0) {
					regs[regid].read();
				}
			}
		} else if (type == iarg_imm) {
			ADDRINT val = va_arg(lst, ADDRINT);
			UINT32 dsigned = va_arg(lst, UINT32);

			if (dsigned) {
				int_bitwidth[0] = max(int_bitwidth[0], get_int_bitwidth((INT64)val));
			} else {
				int_bitwidth[0] = max(int_bitwidth[0], get_uint_bitwidth((UINT64)val));
			}
		} else if (type == iarg_mem) {
			UINT32 is_read = va_arg(lst, UINT32);
			va_arg(lst, UINT32);
			ADDRINT addr = va_arg(lst, ADDRINT);
			ADDRINT size = va_arg(lst, ADDRINT);
			if (is_read) {
				instr->read(addr);
				INT64 val = memory_getvalue(addr, size);
				int_bitwidth[0] = max(int_bitwidth[0], get_int_bitwidth(val));
				is_mem = true;
			}
		}
	}
	va_end(lst);

	// Iterate through the destination operands
	va_start(lst, ops);
	for (UINT32 i = 0; i < ops; i++) {
		UINT32 type = va_arg(lst, UINT32);
		if (type == iarg_reg) {
			int regid = va_arg(lst, INT32);
			va_arg(lst, UINT32);
			UINT32 is_write = va_arg(lst, UINT32);
			va_arg(lst, UINT32);
			va_arg(lst, UINT32);
			va_arg(lst, UINT32);
			va_arg(lst, UINT32);

			if (is_write) {
				if (regid >= 0) {
					regs[regid].write();
				}
			}
		} else if (type == iarg_imm) {
			va_arg(lst, ADDRINT);
			va_arg(lst, UINT32);
			va_arg(lst, UINT32);
		} else if (type == iarg_mem) {
			va_arg(lst, UINT32);
			UINT32 is_write = va_arg(lst, UINT32);
			va_arg(lst, ADDRINT);
			va_arg(lst, ADDRINT);
			if (is_write) {
				is_mem = true;
			}
		}
	}
	va_end(lst);

	for (UINT32 i = 0; i < max_count; i++)
	{
		if (int_bitwidth[i] >= 0)
			opcode->int_bitwidth[int_bitwidth[i]]++;
		if (i < 32 && sig_lbitwidth[i] >= 0)
			opcode->sig_lbitwidth[sig_lbitwidth[i]]++;
		if (i < 32 && sig_mbitwidth[i] >= 0)
			opcode->sig_mbitwidth[sig_mbitwidth[i]]++;
		if (i < 32 && exp_bitwidth[i] >= 0)
			opcode->exp_bitwidth[exp_bitwidth[i]]++;
	}

	ins_count++;
	opcode->count++;
	instr->count++;
	if (is_mem)
		opcode->mem_count++;
}

static void instruction (INS ins, void *v)
{
	string opcode = INS_Mnemonic(ins);
	ADDRINT address = INS_Address(ins);
	xed_decoded_inst_t *xed = INS_XedDec(ins);
	if (opcode.empty())
		return;

	oprecord &record = opmap[opcode];
	insrecord &arecord = insmap[address];
	IARGLIST args = IARGLIST_Alloc();

	record.category = CATEGORY_StringShort(INS_Category(ins));

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
		INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_CONTEXT, IARG_ADDRINT, &record, IARG_ADDRINT, &arecord, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	} else {
		INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_CONTEXT, IARG_ADDRINT, &record, IARG_ADDRINT, &arecord, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	}

	IARGLIST_Free(args);
}

static void on_init() {
	for (int i = 0; i < 84; i++)
		regs[i].type = reg_types + reg_type(i);
}

static void on_finish (INT32 code, void *v)
{
	fprintf(logfp, "writing data\n");

	// Opcode Data
	FILE *fp = fopen(opname, "w");
	// Header
	fprintf(fp, "Category\tOpcode\tExecution Count\tMemory Count\tInteger Bitwidth Execution Count");
	for (int i = 0; i < 63; i++)
		fprintf(fp, "\t");
	fprintf(fp, "\tSignificand Bitwidth Execution Count");
	for (int i = 0; i < 112; i++)
		fprintf(fp, "\t");
	fprintf(fp, "\tExponent Bitwidth Execution Count");
	for (int i = 0; i < 15; i++)
		fprintf(fp, "\t");
	fprintf(fp, "\n");

	// Data
	for (std::map<string,oprecord>::iterator ite = opmap.begin(); ite != opmap.end(); ite++) {
		oprecord &rec = ite->second;

		fprintf(fp, "%s\t%s\t%lu\t%lu", rec.category.c_str(), ite->first.c_str(), rec.count, rec.mem_count);

		for (int i = 0; i < 65; i++) {
			fprintf(fp, "\t%lu", rec.int_bitwidth[i]);
		}

		for (int i = 0; i < 53; i++) {
			fprintf(fp, "\t%lu", rec.sig_lbitwidth[i]);
		}

		for (int i = 0; i < 53; i++) {
			fprintf(fp, "\t%lu", rec.sig_mbitwidth[i]);
		}

		for (int i = 0; i < 17; i++) {
			fprintf(fp, "\t%lu", rec.exp_bitwidth[i]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);

	// Instruction Data
	fp = fopen(insname, "w");
	// Header
	fprintf(fp, "Address\tExecution Count\tTotal Read Count\tIndividual Read Counts\n");

	vector<pair<UINT64, UINT64> > addr;
	for (std::map<ADDRINT, insrecord>::iterator ite = insmap.begin(); ite != insmap.end(); ite++) {
		insrecord &rec = ite->second;
		UINT64 total = 0;

		for (std::map<UINT64, UINT64>::iterator i = rec.read_count.begin(); i != rec.read_count.end(); i++) {
			total += i->second;
			addr.push_back(pair<UINT64, UINT64>(i->second, i->first));
		}
		std::sort(addr.rbegin(), addr.rend());

		fprintf(fp, "%lu\t%lu\t%lu", ite->first, ite->second.count, total);
		for (int i = 0; i < min((int)addr.size(), 32); i++) {
			fprintf(fp, "\t%lu,%lu", addr[i].second, addr[i].first);
		}

		fprintf(fp, "\n");
		addr.clear();
	}
	fclose(fp);

	// Register Fanout Data
	fp = fopen(fanoutname, "w");
	fprintf(fp, "Fanout\tRX\tSEG\tCRX\tDRX\tKX\tTRX\tMMX\n");
	UINT64 fanout = 0;
	UINT64 count[7];
	map<UINT64, UINT64>::iterator regi[7];
	for (int i = 0; i < 7; i++)
		regi[i] = reg_types[i].fanout_distribution.begin();

	while (fanout != (UINT64)-1)
	{
		fanout = (UINT64)-1;
		for (int i = 0; i < 7; i++)
			if (regi[i] != reg_types[i].fanout_distribution.end() && regi[i]->first < fanout)
				fanout = regi[i]->first;
		
		for (int i = 0; i < 7; i++) {
			if (regi[i] == reg_types[i].fanout_distribution.end())
				count[i] = 0;
			else if (regi[i]->first == fanout) {
				count[i] = regi[i]->second;
				regi[i]++;
			} else
				count[i] = 0;
		}
		
		fprintf(fp, "%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n", fanout, count[0], count[1], count[2], count[3], count[4], count[5], count[6]);
	}
	fclose(fp);

	// Register Lifespan Data
	fp = fopen(agename, "w");
	fprintf(fp, "Age\tRX\tSEG\tCRX\tDRX\tKX\tTRX\tMMX\n");
	UINT64 age = 0;
	for (int i = 0; i < 7; i++)
		regi[i] = reg_types[i].age_distribution.begin();

	while (age != (UINT64)-1)
	{
		age = (UINT64)-1;
		for (int i = 0; i < 7; i++)
			if (regi[i] != reg_types[i].age_distribution.end() && regi[i]->first < age)
				age = regi[i]->first;
		
		for (int i = 0; i < 7; i++) {
			if (regi[i] == reg_types[i].age_distribution.end())
				count[i] = 0;
			else if (regi[i]->first == age) {
				count[i] = regi[i]->second;
				regi[i]++;
			} else
				count[i] = 0;
		}
		
		fprintf(fp, "%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n", age, count[0], count[1], count[2], count[3], count[4], count[5], count[6]);
	}
	fclose(fp);

	fprintf(logfp, "finish %d\n", code);
	fclose(logfp);
}

int main (int argc, char *argv[])
{
	if(PIN_Init(argc, argv)) {
		fprintf(stderr, "command line error\n");
		return 1;
	}

	logfp = fopen(logname, "w");

	on_init();

	PIN_InitSymbols();

	PIN_AddFiniFunction(on_finish, 0);
	IMG_AddInstrumentFunction(img_load, NULL);
	IMG_AddUnloadFunction(img_unload, NULL);
	INS_AddInstrumentFunction(instruction, NULL);

	PIN_StartProgram(); // Never returns
	return 0;
}
