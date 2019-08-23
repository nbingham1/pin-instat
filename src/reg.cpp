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

reg_t::reg_t()
{
	reads = 0;
	writes = 0;
	last_read = 0;
	last_instr = 0;
}

reg_t::~reg_t()
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

void register_record_t::write(int register_id, uint64_t instr_count, FILE *log) {
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

		core::table<register_count_t>::iterator i;

		register_count_t *temp = register_fanout.get(fanout, log);
		temp->count[register_class]++;

		temp = register_age.get(age, log);
		temp->count[register_class]++;

		regs[register_id].writes++;
		regs[register_id].last_read = regs[register_id].reads;
		regs[register_id].last_instr = instr_count;
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

