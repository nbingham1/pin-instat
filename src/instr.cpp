#include "instr.h"

memory_key_t::memory_key_t()
{
	instr = 0;
	mem = 0;
}

memory_key_t::memory_key_t(UINT64 instr, UINT64 mem)
{
	this->instr = instr;
	this->mem = mem;
}

memory_key_t::~memory_key_t()
{
}

memory_value_t::memory_value_t()
{
	read = 0;
	write = 0;
}

memory_value_t::memory_value_t(UINT64 read, UINT64 write)
{
	this->read = read;
	this->write = write;
}

memory_value_t::~memory_value_t()
{
}

bool operator<(memory_key_t k0, memory_key_t k1)
{
	return k0.instr < k1.instr ||
				(k0.instr == k1.instr && k0.mem < k1.mem);
}

bool operator>(memory_key_t k0, memory_key_t k1)
{
	return k0.instr > k1.instr ||
				(k0.instr == k1.instr && k0.mem > k1.mem);
}

bool operator<=(memory_key_t k0, memory_key_t k1)
{
	return k0.instr < k1.instr ||
				(k0.instr == k1.instr && k0.mem <= k1.mem);
}

bool operator>=(memory_key_t k0, memory_key_t k1)
{
	return k0.instr > k1.instr ||
				(k0.instr == k1.instr && k0.mem >= k1.mem);
}

bool operator==(memory_key_t k0, memory_key_t k1)
{
	return k0.instr == k1.instr && k0.mem == k1.mem;
}

bool operator!=(memory_key_t k0, memory_key_t k1)
{
	return k0.instr != k1.instr || k0.mem != k1.mem;
}

UINT64 update(UINT64 a, UINT64 b)
{
	return a+b;
}

memory_value_t update_mem(memory_value_t a, memory_value_t b)
{
	a.read += b.read;
	a.write += b.write;
	return a;
}

instruction_record_t::instruction_record_t()
{
	total = 0;
}

instruction_record_t::instruction_record_t(const char *mem, const char *exe) : memory(mem), execution(exe)
{
	total = 0;
}

instruction_record_t::~instruction_record_t()
{
}

void instruction_record_t::finish(FILE *log)
{
	memory.save(log);
	execution.save(log);
}

void instruction_record_t::exec(UINT64 instr, FILE *log)
{
	total++;
	UINT64 *count = execution.get(instr, log);
	(*count)++;
}

void instruction_record_t::read(UINT64 instr, UINT64 mem, FILE *log)
{
	memory_value_t *value = memory.get(memory_key_t(instr, mem), log);
	value->read++;
}

void instruction_record_t::write(UINT64 instr, UINT64 mem, FILE *log)
{
	memory_value_t *value = memory.get(memory_key_t(instr, mem), log);
	value->write++;
}

