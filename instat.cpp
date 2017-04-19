#include <stdio.h>
#include <pin.H>
#include <map>
#include <string>
#include <algorithm>
#include <locale>

#ifdef _MSC_VER
# define PxPTR "%Ix"
#else
# ifdef TARGET_IA32
#  define PxPTR "%x"
# else
#  define PxPTR "%lx"
# endif
#endif

struct insrecord {
	insrecord() {
		count = 0;
		for (int i = 0; i < 64; i++)
			bitwidth_count[i] = 0;
	}

	~insrecord() {}

	string category;
	unsigned int bitwidth_count[64];
	int count;
};

struct addrrecord {
	addrrecord() {
		count = 0;
	}

	int count;
};

enum
{
	iarg_reg = 0,
	iarg_mem = 1,
	iarg_imm = 2,
	iarg_br = 3
};

const char *logname = "instat.log";
const char *tsvname = "instat.tsv";
FILE *logfp;
std::map<ADDRINT,addrrecord> addrmap;
std::map<string,insrecord> insmap;
std::map<ADDRINT,string> symbols;
std::map<ADDRINT,std::pair<ADDRINT,string> > imgs;

static void img_load (IMG img, void *v)
{
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

static inline bool REG_is_integer (REG reg) {
	return (reg >= REG_RBASE && reg < REG_MM_BASE);
}

unsigned int get_bitwidth(ADDRINT val)
{
	long long int test = val;
	unsigned int bitwidth = 0;
	while (test != 0 && test != -1)
	{
		test >>= 1;
		bitwidth++;
	}
	return bitwidth;
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

static void on_ins (insrecord *rec, addrrecord *arec, UINT32 ops, ...)
{
	va_list lst;
	UINT32 bitwidth = 0;
	ADDRINT addr, size, val;

	va_start(lst, ops);
	for (UINT32 i = 0; i < ops; i++) {
		UINT32 type = va_arg(lst, UINT32);
		switch (type) {
		case iarg_reg:
		case iarg_imm:
			bitwidth = max(bitwidth, get_bitwidth(va_arg(lst, ADDRINT)));
			break;
		case iarg_mem:
			addr = va_arg(lst, ADDRINT);
			size = va_arg(lst, ADDRINT);
			val = memory_getvalue(addr, size);
			bitwidth = max(bitwidth, get_bitwidth(val));
			break;
		case iarg_br:
		default:
			break;
		}
	}
	va_end(lst);
	rec->bitwidth_count[bitwidth]++;
	rec->count++;
	arec->count++;
}

static void instruction (INS ins, void *v)
{
	string opcode = INS_Mnemonic(ins);
	ADDRINT address = INS_Address(ins);
	if (opcode.empty())
		return;

	insrecord &record = insmap[opcode];
	addrrecord &arecord = addrmap[address];
	IARGLIST args = IARGLIST_Alloc();

	record.category = CATEGORY_StringShort(INS_Category(ins));

	UINT32 ops = 0;
	for (UINT32 i = 0; i < INS_OperandCount(ins); i++) {
		if (INS_OperandIsReg(ins, i)) {
			if (INS_OperandRead(ins, i)) {
				if (REG_is_integer(INS_OperandReg(ins, i))) {
					IARGLIST_AddArguments(args, IARG_UINT32, iarg_reg,
						IARG_REG_VALUE, INS_OperandReg(ins, i),
						IARG_END);
					ops++;
				}
			}
		} else if (INS_OperandIsImmediate(ins, i)) {
			IARGLIST_AddArguments(args, IARG_UINT32, iarg_imm, IARG_ADDRINT, INS_OperandImmediate(ins, i), IARG_END);
			ops++;
		}
	}

	for (UINT32 i = 0; i < INS_MemoryOperandCount(ins); i++) {
		if (INS_MemoryOperandIsRead(ins, i)) {
			IARGLIST_AddArguments(args, IARG_UINT32, iarg_mem,
				IARG_MEMORYOP_EA, i,
				IARG_MEMORYREAD_SIZE,
				IARG_END);
			ops++;
		}
	}

	if (INS_IsPredicated(ins)) {
		INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_ADDRINT, &record, IARG_ADDRINT, &arecord, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	} else {
		INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_ADDRINT, &record, IARG_ADDRINT, &arecord, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	}

	IARGLIST_Free(args);
}

static void on_finish (INT32 code, void *v)
{
	fprintf(logfp, "finish %d\n", code);
	FILE *fp = fopen(tsvname, "w");
	for(std::map<string,insrecord>::iterator ite = insmap.begin(); ite != insmap.end(); ite++) {
		struct insrecord &rec = ite->second;

		fprintf(fp, "%s\t%s\t%d", rec.category.c_str(), ite->first.c_str(), rec.count);

		for (int i = 0; i < 64; i++) {
			fprintf(fp, "\t%u", rec.bitwidth_count[i]);
		}

		fprintf(fp, "\n");
	}
	fprintf(fp, "\n\n");

	for (std::map<ADDRINT, addrrecord>::iterator ite = addrmap.begin(); ite != addrmap.end(); ite++) {
		fprintf(fp, PxPTR "\t%d\n", ite->first, ite->second.count);
	}

	fclose(fp);
	fclose(logfp);
}

int main (int argc, char *argv[])
{
	if(PIN_Init(argc, argv)) {
		fprintf(stderr, "command line error\n");
		return 1;
	}

	logfp = fopen(logname, "w");

	PIN_InitSymbols();

	PIN_AddFiniFunction(on_finish, 0);
	IMG_AddInstrumentFunction(img_load, NULL);
	IMG_AddUnloadFunction(img_unload, NULL);
	INS_AddInstrumentFunction(instruction, NULL);

	PIN_StartProgram(); // Never returns
	return 0;
}
