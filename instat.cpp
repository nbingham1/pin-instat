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
		mem_count = 0;
		for (int i = 0; i < 64; i++)
			bitwidth_count[i] = 0;
	}

	~insrecord() {}

	string category;
	unsigned int bitwidth_count[64];
	int count;
	int mem_count;
};

struct addrrecord {
	addrrecord() {
		count = 0;
	}

	int count;
};

UINT64 next_id = 0;
std::map<UINT64, int> memlife;

struct depgraph {
	depgraph() {}
	~depgraph() {}

	map<UINT64, UINT64> g;

	void merge(const depgraph &d) {
		map<UINT64, UINT64>::const_iterator i;
		map<UINT64, UINT64>::iterator j;
		for (i = d.g.begin(); i != d.g.end(); i++) {
			j = g.find(i->first);
			if (j != g.end())
				j->second = max(j->second, i->second);
			else
				g.insert(*j);
		}
	}

	void clear() {
		g.clear();
	}

	void set(const depgraph &d) {
		g = d.g;
	}

	void inc() {
		map<UINT64, UINT64>::iterator i;
		for (i = g.begin(); i != g.end(); i++)
			i->second++;
	}

	void add() {
		g.insert(pair<UINT64, UINT64>(next_id, 1));
		next_id++;
	}

	void save() {
		map<UINT64, UINT64>::iterator i;
		UINT64 m = 0;
		for (i = g.begin(); i != g.end(); i++) {
			if (i->second > m)
				m = i->second;
		}

		if (m > 0) {
			map<UINT64, int>::iterator j = memlife.find(m);
			if (j == memlife.end())
				memlife.insert(pair<UINT64, int>(m, 1));
			else
				j->second++;
		}
	}
};

std::map<int, int> reglife;

struct regrecord {
	regrecord() {
		count = 0;
	}

	~regrecord() {}

	int count;
	depgraph length;

	void read() {
		count++;
	}

	void write() {
		if (count >= 0) {
			map<int, int>::iterator j = reglife.find(count);
			if (j == reglife.end())
				reglife.insert(pair<int, int>(count, 1));
			else
				j->second++;
		}
		count = 0;
	}
};

enum
{
	iarg_reg = 0,
	iarg_mem = 1,
	iarg_imm = 2,
	iarg_br = 3
};

const char *logname = "instat.log";
const char *insname = "instat.tsv";
const char *addrname = "addrstat.tsv";
const char *regname = "regstat.tsv";
const char *memname = "memstat.tsv";
FILE *logfp;
regrecord regs[16];
std::map<ADDRINT,addrrecord> addrmap;
std::map<string,insrecord> insmap;
std::map<ADDRINT,string> symbols;
std::map<ADDRINT,std::pair<ADDRINT,string> > imgs;

int reg_id(string reg) {
	if (reg.size() >= 2) {
		if (reg[0] == 'a' || reg[1] == 'a')
			return 0;
		else if (reg[0] == 'b' || reg[1] == 'b')
			return 1;
		else if (reg[0] == 'c' || reg[1] == 'c')
			return 2;
		else if (reg[0] == 'd' || reg[1] == 'd')
			return 3;
		else if (strncmp(reg.c_str(), "si", 2) == 0 ||
						 strncmp(reg.c_str()+1, "si", 2) == 0)
			return 4;
		else if (strncmp(reg.c_str(), "di", 2) == 0 ||
						 strncmp(reg.c_str()+1, "di", 2) == 0)
			return 5;
		else if (strncmp(reg.c_str(), "bp", 2) == 0 ||
						 strncmp(reg.c_str()+1, "bp", 2) == 0)
			return 6;
		else if (strncmp(reg.c_str(), "sp", 2) == 0 ||
						 strncmp(reg.c_str()+1, "sp", 2) == 0)
			return 7;
		else if (strncmp(reg.c_str(), "r8", 2) == 0)
			return 8;
		else if (strncmp(reg.c_str(), "r9", 2) == 0)
			return 9;
		else if (strncmp(reg.c_str(), "r10", 2) == 0)
			return 10;
		else if (strncmp(reg.c_str(), "r11", 2) == 0)
			return 11;
		else if (strncmp(reg.c_str(), "r12", 2) == 0)
			return 12;
		else if (strncmp(reg.c_str(), "r13", 2) == 0)
			return 13;
		else if (strncmp(reg.c_str(), "r14", 2) == 0)
			return 14;
		else if (strncmp(reg.c_str(), "r15", 2) == 0)
			return 15;
	}

	return -1;
}

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
	UINT32 is_read = 0;
	UINT32 is_write = 0;
	regrecord *rrec = NULL;
	bool is_mem = false;

	depgraph g;

	// Iterate through the source operands
	va_start(lst, ops);
	for (UINT32 i = 0; i < ops; i++) {
		UINT32 type = va_arg(lst, UINT32);
		switch (type) {
		case iarg_reg:
			rrec = va_arg(lst, regrecord*);
			is_read = va_arg(lst, UINT32);
			is_write = va_arg(lst, UINT32);
			val = va_arg(lst, ADDRINT);
			if (is_read) {
				bitwidth = max(bitwidth, get_bitwidth(val));
				if (rrec != NULL) {
					rrec->read();
					g.merge(rrec->length);
				}
			}
			break;
		case iarg_imm:
			val = va_arg(lst, ADDRINT);
			bitwidth = max(bitwidth, get_bitwidth(val));
			break;
		case iarg_mem:
			is_read = va_arg(lst, UINT32);
			is_write = va_arg(lst, UINT32);
			addr = va_arg(lst, ADDRINT);
			size = va_arg(lst, ADDRINT);
			if (is_read) {
				val = memory_getvalue(addr, size);
				bitwidth = max(bitwidth, get_bitwidth(val));
				g.add();
				is_mem = true;
			}
			break;
		default:
			break;
		}
	}
	va_end(lst);

	g.inc();

	// Iterate through the destination operands
	va_start(lst, ops);
	for (UINT32 i = 0; i < ops; i++) {
		UINT32 type = va_arg(lst, UINT32);
		switch (type) {
		case iarg_reg:
			rrec = va_arg(lst, regrecord*);
			is_read = va_arg(lst, UINT32);
			is_write = va_arg(lst, UINT32);
			val = va_arg(lst, ADDRINT);
			if (is_write) {
				if (rrec != NULL) {
					rrec->write();
					rrec->length.set(g);
				}
			}
			break;
		case iarg_imm:
			va_arg(lst, ADDRINT);
			break;
		case iarg_mem:
			is_read = va_arg(lst, UINT32);
			is_write = va_arg(lst, UINT32);
			addr = va_arg(lst, ADDRINT);
			size = va_arg(lst, ADDRINT);
			if (is_write) {
				g.save();
				is_mem = true;
			}
			break;
		default:
			break;
		}
	}
	va_end(lst);

	rec->bitwidth_count[bitwidth]++;
	rec->count++;
	arec->count++;
	if (is_mem)
		rec->mem_count++;
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
			BOOL read = INS_OperandRead(ins, i);
			BOOL write = INS_OperandWritten(ins, i);
			int regname = reg_id(REG_StringShort(INS_OperandReg(ins, i)));
			regrecord *rrecord = NULL;
			if (regname >= 0 && regname < 16)
				rrecord = regs + regname;	

			if (REG_is_integer(INS_OperandReg(ins, i))) {
				IARGLIST_AddArguments(args, IARG_UINT32, iarg_reg,
					IARG_ADDRINT, rrecord,
					IARG_UINT32, (UINT32)read,
					IARG_UINT32, (UINT32)write,
					IARG_REG_VALUE, INS_OperandReg(ins, i),
					IARG_END);
				ops++;
			}
		} else if (INS_OperandIsImmediate(ins, i)) {
			IARGLIST_AddArguments(args, IARG_UINT32, iarg_imm, IARG_ADDRINT, INS_OperandImmediate(ins, i), IARG_END);
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
		INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_ADDRINT, &record, IARG_ADDRINT, &arecord, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	} else {
		INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(on_ins), IARG_ADDRINT, &record, IARG_ADDRINT, &arecord, IARG_UINT32, ops, IARG_IARGLIST, args, IARG_END);
	}

	IARGLIST_Free(args);
}

static void on_finish (INT32 code, void *v)
{
	fprintf(logfp, "finish %d\n", code);
	FILE *fp = fopen(insname, "w");
	for (std::map<string,insrecord>::iterator ite = insmap.begin(); ite != insmap.end(); ite++) {
		struct insrecord &rec = ite->second;

		fprintf(fp, "%s\t%s\t%d\t%d", rec.category.c_str(), ite->first.c_str(), rec.count, rec.mem_count);

		for (int i = 0; i < 64; i++) {
			fprintf(fp, "\t%u", rec.bitwidth_count[i]);
		}

		fprintf(fp, "\n");
	}
	fclose(fp);

	fp = fopen(addrname, "w");
	for (std::map<ADDRINT, addrrecord>::iterator ite = addrmap.begin(); ite != addrmap.end(); ite++) {
		fprintf(fp, PxPTR "\t%d\n", ite->first, ite->second.count);
	}
	fclose(fp);

	fp = fopen(regname, "w");
	for (std::map<int, int>::iterator i = reglife.begin(); i != reglife.end(); i++)
		fprintf(fp, "%d\t%d\n", i->first, i->second);
	fclose(fp);

	fp = fopen(memname, "w");
	for (std::map<UINT64, int>::iterator i = memlife.begin(); i != memlife.end(); i++)
		fprintf(fp, "%lu\t%d\n", i->first, i->second);
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
