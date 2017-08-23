#include "instr.h"

memory_key_t::memory_key_t()
{
	instr = 0;
	mem = 0;
}

memory_key_t::memory_key_t(uint64_t instr, uint64_t mem)
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

memory_value_t::memory_value_t(uint64_t read, uint64_t write)
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

bool operator<(memory_value_t k0, memory_value_t k1)
{
	return (k0.read + k0.write) < (k1.read + k1.write);
}

bool operator>(memory_value_t k0, memory_value_t k1)
{
	return (k0.read + k0.write) > (k1.read + k1.write);
}

bool operator<=(memory_value_t k0, memory_value_t k1)
{
	return (k0.read + k0.write) <= (k1.read + k1.write);
}

bool operator>=(memory_value_t k0, memory_value_t k1)
{
	return (k0.read + k0.write) >= (k1.read + k1.write);
}

bool operator==(memory_value_t k0, memory_value_t k1)
{
	return (k0.read + k0.write) == (k1.read + k1.write);
}

bool operator!=(memory_value_t k0, memory_value_t k1)
{
	return (k0.read + k0.write) != (k1.read + k1.write);
}

uint64_t update(uint64_t a, uint64_t b)
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

void instruction_record_t::exec(uint64_t instr, FILE *log)
{
	total++;
	uint64_t *count = execution.get(instr, log);
	(*count)++;
}

void instruction_record_t::read(uint64_t instr, uint64_t mem, FILE *log)
{
	memory_value_t *value = memory.get(memory_key_t(instr, mem), log);
	value->read++;
}

void instruction_record_t::write(uint64_t instr, uint64_t mem, FILE *log)
{
	memory_value_t *value = memory.get(memory_key_t(instr, mem), log);
	value->write++;
}

