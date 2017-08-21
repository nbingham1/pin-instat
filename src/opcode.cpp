#include "opcode.h"

extern "C" {
#include <xed-interface.h>
}

opcode_t::opcode_t()
{
	category = -1;
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

opcode_t::opcode_t(int category)
{
	this->category = category;
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

opcode_t::~opcode_t()
{
}

opcode_t update_opcode(opcode_t a, opcode_t b)
{
	a.count += b.count;
	a.mem_count += b.mem_count;
	for (int i = 0; i < 65; i++)
		a.int_bitwidth[i] += b.int_bitwidth[i];
	for (int i = 0; i < 53; i++) {
		a.sig_lbitwidth[i] += b.sig_lbitwidth[i];
		a.sig_mbitwidth[i] += b.sig_mbitwidth[i];
	}
	for (int i = 0; i < 17; i++)
		a.exp_bitwidth[i] += b.exp_bitwidth[i];
	return a;
}

uint64_t get_ufield(uint8_t *val, uint32_t start, uint32_t length)
{
	uint64_t sect = *(uint64_t*)(val + (start>>3));
	uint32_t off = start & 7;
	sect <<= 64 - length - off;
	sect >>= 64 - length;
	return sect;
}

int64_t get_sfield(uint8_t *val, uint32_t start, uint32_t length)
{
	int64_t sect = *(int64_t*)(val + (start>>3));
	uint32_t off = start & 7;
	sect <<= 64 - length - off;
	sect >>= 64 - length;
	return sect;
}

int get_int_bitwidth(int64_t val) {
	int bitwidth = 0;	
	int64_t test = val;
	while (test != 0 && test != -1) {
		test >>= 1;
		bitwidth++;
	}
	return bitwidth;
}

int get_uint_bitwidth(uint64_t val) {
	int bitwidth = 0;	
	uint64_t test = val;
	while (test != 0) {
		test >>= 1;
		bitwidth++;
	}
	return bitwidth;
}

int get_msbf_bitwidth(uint64_t val, uint32_t width) {
	int bitwidth = 0;	
	uint64_t test = val << (64-width);
	while (test != 0) {
		test <<= 1;
		bitwidth++;
	}
	return bitwidth;
}

int get_sig_lbitwidth(uint8_t *val, uint32_t type) {
	uint32_t sign;
	uint32_t width;
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

	int64_t field = get_sfield(val, 0, width);
	if (sign)
		field = -field;	

	return get_int_bitwidth(field);
}

int get_sig_mbitwidth(uint8_t *val, uint32_t type) {
	uint32_t sign;
	uint32_t width;
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

	int64_t field = get_sfield(val, 0, width);
	if (sign)
		field = -field;

	return get_msbf_bitwidth(field, width);
}


int get_exp_bitwidth(uint8_t *val, uint32_t type) {
	int64_t field = 0;
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

uintptr_t memory_getvalue(uintptr_t addr, uintptr_t size)
{
	uintptr_t val;
	switch (size) {
		case 1: val = *(uint8_t *)addr; break;
		case 2: val = *(uint16_t *)addr; break;
		case 4: val = *(uint32_t *)addr; break;
#ifdef TARGET_IA32E
		case 8: val = *(uint64_t *)addr; break;
#endif
		default: val = *(uintptr_t *)addr;
	}
	return val;
}

int get_category(uint32_t opcode)
{
	switch (opcode)
	{
	case XED_ICLASS_PMINUB:
	case XED_ICLASS_CMP:
	case XED_ICLASS_VPCMPISTRI:
	case XED_ICLASS_PCMPISTRI:
	case XED_ICLASS_PCMPGTB:
	case XED_ICLASS_VPCMPGTB:
	case XED_ICLASS_VPCMPEQB:
	case XED_ICLASS_PCMPEQD:
	case XED_ICLASS_PCMPEQB:
	case XED_ICLASS_PMAXUB:
	case XED_ICLASS_REPNE_SCASB:
	case XED_ICLASS_REPE_CMPSB:
	case XED_ICLASS_BSR:
	case XED_ICLASS_BSF:
	case XED_ICLASS_TZCNT:
	case XED_ICLASS_SETZ:
	case XED_ICLASS_SETLE:
	case XED_ICLASS_SETNLE:
	case XED_ICLASS_SETB:
	case XED_ICLASS_SETL:
	case XED_ICLASS_SETP:
	case XED_ICLASS_SETNL:
	case XED_ICLASS_SETNB:
	case XED_ICLASS_SETNP:
	case XED_ICLASS_SETNBE:
	case XED_ICLASS_SETBE:
	case XED_ICLASS_SETNZ:
	case XED_ICLASS_BT:
	case XED_ICLASS_BTS:
	case XED_ICLASS_PTEST:
	case XED_ICLASS_TEST:
	case XED_ICLASS_SHL:
	case XED_ICLASS_SHR:
	case XED_ICLASS_SHLD:
	case XED_ICLASS_SHRD:
	case XED_ICLASS_VPSLLDQ:
	case XED_ICLASS_PSRLDQ:
	case XED_ICLASS_PSLLDQ:
	case XED_ICLASS_ROL:
	case XED_ICLASS_ROR:
	case XED_ICLASS_SAR:
	case XED_ICLASS_ADD:
	case XED_ICLASS_ADC:
	case XED_ICLASS_ADD_LOCK:
	case XED_ICLASS_XADD_LOCK:
	case XED_ICLASS_SUB:
	case XED_ICLASS_SBB:
	case XED_ICLASS_PSUBB:
	case XED_ICLASS_VPSUBB:
	case XED_ICLASS_SUB_LOCK:
	case XED_ICLASS_MUL:
	case XED_ICLASS_IMUL:
	case XED_ICLASS_IDIV:
	case XED_ICLASS_DIV:
	case XED_ICLASS_AND:
	case XED_ICLASS_PAND:
	case XED_ICLASS_VPAND:
	case XED_ICLASS_VPANDN:
	case XED_ICLASS_XOR:
	case XED_ICLASS_PXOR:
	case XED_ICLASS_VPXOR:
	case XED_ICLASS_OR:
	case XED_ICLASS_POR:
	case XED_ICLASS_VPOR:
	case XED_ICLASS_OR_LOCK:
	case XED_ICLASS_INC:
	case XED_ICLASS_DEC:
	case XED_ICLASS_DEC_LOCK:
	case XED_ICLASS_NOT:
	case XED_ICLASS_NEG:
		return ARITH_INT;
	case XED_ICLASS_MINSS:
	case XED_ICLASS_MINSD:
	case XED_ICLASS_MAXSS:
	case XED_ICLASS_MAXSD:
	case XED_ICLASS_CMPSS:
	case XED_ICLASS_VCMPSD:
	case XED_ICLASS_CMPSD_XMM:
	case XED_ICLASS_FUCOMI:
	case XED_ICLASS_UCOMISS:
	case XED_ICLASS_UCOMISD:
	case XED_ICLASS_VUCOMISD:
	case XED_ICLASS_ADDSS:
	case XED_ICLASS_ADDPD:
	case XED_ICLASS_VADDSD:
	case XED_ICLASS_ADDSD:
	case XED_ICLASS_FSUB:
	case XED_ICLASS_FSUBP:
	case XED_ICLASS_FSUBR:
	case XED_ICLASS_FSUBRP:
	case XED_ICLASS_SUBSS:
	case XED_ICLASS_VSUBSD:
	case XED_ICLASS_SUBSD:
	case XED_ICLASS_FMUL:
	case XED_ICLASS_FMULP:
	case XED_ICLASS_MULSD:
	case XED_ICLASS_MULSS:
	case XED_ICLASS_VMULSD:
	case XED_ICLASS_MULPD:
	case XED_ICLASS_FDIV:
	case XED_ICLASS_FDIVP:
	case XED_ICLASS_FDIVRP:
	case XED_ICLASS_VDIVSD:
	case XED_ICLASS_DIVSD:
	case XED_ICLASS_DIVSS:
	case XED_ICLASS_ROUNDSS:
	case XED_ICLASS_ROUNDSD:
	case XED_ICLASS_SQRTSS:
	case XED_ICLASS_SQRTSD:
	case XED_ICLASS_ANDPS:
	case XED_ICLASS_ANDNPS:
	case XED_ICLASS_ANDPD:
	case XED_ICLASS_VANDPD:
	case XED_ICLASS_VANDNPD:
	case XED_ICLASS_ANDNPD:
	case XED_ICLASS_ORPS:
	case XED_ICLASS_ORPD:
	case XED_ICLASS_VORPD:
	case XED_ICLASS_XORPS:
	case XED_ICLASS_XORPD:
	case XED_ICLASS_VXORPD:
	case XED_ICLASS_FCHS:
		return ARITH_REAL;
	case XED_ICLASS_REP_MOVSD:
	case XED_ICLASS_REP_MOVSQ:
	case XED_ICLASS_REP_STOSB:
	case XED_ICLASS_REP_STOSD:
	case XED_ICLASS_REP_STOSQ:
	case XED_ICLASS_CMOVS:
	case XED_ICLASS_CMOVZ:
	case XED_ICLASS_CMOVL:
	case XED_ICLASS_CMOVB:
	case XED_ICLASS_CMOVBE:
	case XED_ICLASS_CMOVLE:
	case XED_ICLASS_CMOVNL:
	case XED_ICLASS_CMOVNB:
	case XED_ICLASS_CMOVNZ:
	case XED_ICLASS_CMOVNS:
	case XED_ICLASS_CMOVNBE:
	case XED_ICLASS_CMOVNLE:
	case XED_ICLASS_CMPXCHG_LOCK:
	case XED_ICLASS_CMPXCHG:
	case XED_ICLASS_NOP:
	case XED_ICLASS_MOVZX:
	case XED_ICLASS_MOVSX:
	case XED_ICLASS_MOVSXD:
	case XED_ICLASS_MOV:
	case XED_ICLASS_MOVD:
	case XED_ICLASS_MOVQ:
	case XED_ICLASS_VMOVD:
	case XED_ICLASS_VMOVQ:
	case XED_ICLASS_MOVDQU:
	case XED_ICLASS_MOVDQA:
	case XED_ICLASS_VMOVDQA:
	case XED_ICLASS_VMOVDQU:
	case XED_ICLASS_MOVNTI:
	case XED_ICLASS_LDDQU:
	case XED_ICLASS_PUNPCKLQDQ:
	case XED_ICLASS_PUNPCKLWD:
	case XED_ICLASS_PUNPCKLBW:
	case XED_ICLASS_PSHUFB:
	case XED_ICLASS_PSHUFD:
	case XED_ICLASS_XCHG:
	case XED_ICLASS_BSWAP:
	case XED_ICLASS_PALIGNR:
	case XED_ICLASS_VPALIGNR:
	case XED_ICLASS_PUSH:
	case XED_ICLASS_POP:
		return ROUTE_INT;
	case XED_ICLASS_MOVAPD:
	case XED_ICLASS_MOVHPD:
	case XED_ICLASS_MOVLPD:
	case XED_ICLASS_VMOVAPD:
	case XED_ICLASS_VMOVSD:
	case XED_ICLASS_MOVSD_XMM:
	case XED_ICLASS_MOVSS:
	case XED_ICLASS_MOVAPS:
	case XED_ICLASS_MOVUPS:
	case XED_ICLASS_FXCH:
	case XED_ICLASS_FNSTCW:
	case XED_ICLASS_FST:
	case XED_ICLASS_FSTP:
	case XED_ICLASS_FLD:
	case XED_ICLASS_FLDZ:
	case XED_ICLASS_FLD1:
	case XED_ICLASS_FLDCW:
	case XED_ICLASS_CDQ:
	case XED_ICLASS_CDQE:
	case XED_ICLASS_CWDE:
	case XED_ICLASS_CQO:
	case XED_ICLASS_FILD:
	case XED_ICLASS_CVTSS2SD:
	case XED_ICLASS_VCVTSI2SD:
	case XED_ICLASS_CVTPD2PS:
	case XED_ICLASS_CVTTSS2SI:
	case XED_ICLASS_CVTTSD2SI:
	case XED_ICLASS_VCVTTSD2SI:
	case XED_ICLASS_CVTSI2SS:
	case XED_ICLASS_CVTSD2SS:
	case XED_ICLASS_CVTSI2SD:
	case XED_ICLASS_VUNPCKLPD:
	case XED_ICLASS_UNPCKLPD:
	case XED_ICLASS_SHUFPS:
	case XED_ICLASS_MOVMSKPD:
		return ROUTE_REAL;
	case XED_ICLASS_JZ:
	case XED_ICLASS_JP:
	case XED_ICLASS_JS:
	case XED_ICLASS_JB:
	case XED_ICLASS_JNLE:
	case XED_ICLASS_JNS:
	case XED_ICLASS_JL:
	case XED_ICLASS_JNP:
	case XED_ICLASS_JNZ:
	case XED_ICLASS_JNB:
	case XED_ICLASS_JNL:
	case XED_ICLASS_JMP:
	case XED_ICLASS_JLE:
	case XED_ICLASS_JBE:
	case XED_ICLASS_JNBE:
		return CTRL_JUMP;
	case XED_ICLASS_CALL_NEAR:
	case XED_ICLASS_SYSCALL:
	case XED_ICLASS_RET_NEAR:
	case XED_ICLASS_FWAIT:
	case XED_ICLASS_FNCLEX:
	case XED_ICLASS_LEAVE:
	case XED_ICLASS_SFENCE:
	case XED_ICLASS_PREFETCHNTA:
	case XED_ICLASS_CPUID:
	case XED_ICLASS_UD2:
	case XED_ICLASS_PREFETCHT0:
	case XED_ICLASS_RDTSC:
	case XED_ICLASS_CLD:
	case XED_ICLASS_VLDMXCSR:
	case XED_ICLASS_VSTMXCSR:
	case XED_ICLASS_LDMXCSR:
	case XED_ICLASS_STMXCSR:
	case XED_ICLASS_PMOVMSKB:
	case XED_ICLASS_VPMOVMSKB:
	case XED_ICLASS_XGETBV:
	case XED_ICLASS_LEA:
	default:
		return CTRL_OTHER;
	}
}

