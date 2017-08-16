#include "opcode.h"

table<implier<uint64_t, opcode_t> > opcodes_file("opcodes.tbl", false);
map<uint64_t, opcode_t> opcodes;

void emit_opcode(FILE *op, int category, opcode_t opcode)
{
	fprintf(op, "%d\t%lu\t%lu\t", category, opcode.count, opcode.mem_count);
	for (int j = 0; j < 65; i++)
		fprintf(op, "%lu\n", opcode.int_bitwidth[j]);
	for (int j = 0; j < 53; i++)
		fprintf(op, "%lu\n", opcode.sig_lbitwidth[j]);
	for (int j = 0; j < 53; i++)
		fprintf(op, "%lu\n", opcode.sig_mbitwidth[j]);
	for (int j = 0; j < 17; i++)
		fprintf(op, "%lu\n", opcode.exp_bitwidth[j]);
	fprintf(op, "\n");
}

void emit_counts(FILE *op, opcode_t opcode)
{
	
}

int main()
{
	UINT64 total = 0;
	opcode_t categories[6];
	for (table<implier<uint64_t, opcode_t> >::iterator i = opcodes_file.begin(); i != opcodes_file.end(); i++)
	{
		int category = get_category(i->key);
		opcodes.insert(i->key, i->value);
		total += i->value.count;
		categories[category] = update_opcode(categories[category], i->value);
	}

	FILE *op = fopen("opcodes.tsv", "w");
	fclose(op);

	op = fopen("category.tsv", "w");
	for (int i = 0; i < 6; i++)
	{	
		emit_opcode(op, i, categories[i]);
	}
	fclose(op);
}

