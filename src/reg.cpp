#include "reg.h"

register_count_t::register_count_t()
{
	for (int i = 0; i < 7; i++)
		count[i] = 0;
}

register_count_t::~register_count_t()
{
}

bool operator==(register_count_t r0, register_count_t r1)
{
	for (int i = 0; i < 7; i++)
		if (r0.count[i] != r1.count[i])
			return false;
	return true;
}

bool operator!=(register_count_t r0, register_count_t r1)
{
	for (int i = 0; i < 7; i++)
		if (r0.count[i] != r1.count[i])
			return true;
	return false;
}

register_t::register_t()
{
	reads = 0;
	writes = 0;
	last_read = 0;
	last_instr = 0;
}

register_t::~register_t()
{
}

register_record_t::register_record_t()
{
}

register_record_t::register_record_t(const char *fanout, const char *age) : register_fanout(fanout), register_age(age)
{
}

register_record_t::~register_record_t()
{
}

void register_record_t::finish(FILE *log)
{
	register_fanout.finish(log);
	register_age.finish(log);
}

void register_record_t::read(int register_id) {
	regs[register_id].reads++;
}

void register_record_t::write(int register_id, UINT64 instr_count, FILE *log) {
	int register_class = reg_class(register_id);
	if (register_class >= 0)
	{
		int fanout = regs[register_id].reads - regs[register_id].last_read;
		int age = instr_count - regs[register_id].last_instr;

		register_count_t count;

		// We don't need a several-Gb table to store
		// values we don't care about.
		if (fanout > 999)
			fanout = 999;
		if (age > 999)
			age = 999;

		file::table<register_count_t>::iterator i;

		register_count_t *temp = register_fanout.get(fanout, log);
		temp->count[register_class]++;

		temp = register_age.get(age, log);
		temp->count[register_class]++;

		regs[register_id].writes++;
		regs[register_id].last_read = regs[register_id].reads;
		regs[register_id].last_instr = instr_count;
	}
}

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

int reg_class(int register_id)
{
	if (register_id < 0)
		return -1;
	else if (register_id < 16)
		return 0;
	else if (register_id < 23)
		return 1;
	else if (register_id < 28)
		return 2;
	else if (register_id < 36)
		return 3;
	else if (register_id < 44)
		return 4;
	else if (register_id < 52)
		return 5;
	else if (register_id < 84)
		return 6;
	else
		return -1;
}

int reg_width(REG reg) {
	return _regWidthToBitWidth[REG_Width(reg)];
}

