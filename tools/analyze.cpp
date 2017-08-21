#include "src/opcode.h"
#include "src/table.h"
#include "src/map.h"

#include <ctype.h>

int cat_col(int cat)
{
	switch (cat)
	{
	case ARITH_INT:
	case ROUTE_INT:
		return 0;
	case ARITH_REAL:
	case ROUTE_REAL:
		return 1;
	case CTRL_JUMP:
		return 2;
	case CTRL_OTHER:
		return 3;
	}
	return -1;
}

int main()
{
	FILE *fptr;
	const char *cat_names[] = {
		"Arithmetic Integer",
		"Arithmetic Real",
		"Routing Integer",
		"Routing Real",
		"Control Jump",
		"Control Other"
	};

	{
		file::table<core::implier<uint64_t, opcode_t> > opcodes_file("opcodes.tbl", false);
		core::map<uint64_t, implier<uint64_t, opcode_t> > opcodes;

		uint64_t total = 0;
		opcode_t categories[6];
		for (file::table<core::implier<uint64_t, opcode_t> >::iterator i = opcodes_file.begin(); i != opcodes_file.end(); i++)
		{
			int category = get_category(i->key);
			opcodes.insert(i->value.count, *i);
			total += i->value.count;
			categories[category] = update_opcode(categories[category], i->value);
		}

		{
			fptr = fopen("category_usage.tsv", "w");
			fprintf(fptr, "Category\tInteger\tInteger (Mem)\tReal\tReal (Mem)\tJump\tJump (Mem)\tOther\tOther (Mem)\n");
			const char *col_names[] = {"Arithmetic", "Routing", "Control"};
			opcode_t *cat = &categories[0];
			for (int i = 0; i < 3; i++) {
				fprintf(fptr, "%s\t", col_names[i]);
				if (i == 2)
					fprintf(fptr, "\t\t\t\t");
				for (int j = 0; j < 2; j++) {
					fprintf(fptr, "%lu\t%lu\t",
						cat->count - cat->mem_count,
						cat->mem_count);
					cat++;
				}
				fprintf(fptr, "\n");
			}
			fprintf(fptr, "\n\n");
			fprintf(fptr, "Category\tInteger\tInteger (Mem)\tReal\tReal (Mem)\tJump\tJump (Mem)\tOther\tOther (Mem)\n");
			cat = &categories[0];
			for (int i = 0; i < 3; i++) {
				fprintf(fptr, "%s\t", col_names[i]);
				if (i == 2)
					fprintf(fptr, "\t\t\t\t");
				for (int j = 0; j < 2; j++) {
					fprintf(fptr, "%f\t%f\t",
						(float)(cat->count - cat->mem_count)/(float)total,
						(float)cat->mem_count/(float)total);
					cat++;
				}
				fprintf(fptr, "\n");
			}
			fclose(fptr);	
		}

		{
			fptr = fopen("instruction_usage.tsv", "w");
			fprintf(fptr, "Instruction\tCategory\tTotal\tMemory\tNon-Memory\tTotal\tInteger\tInteger (Mem)\tReal\tReal (Mem)\tJump\tJump (Mem)\tOther\tOther (Mem)\n");
			for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
			{
				int category = get_category(i->value.key);
				int col = cat_col(category);
				opcode_t *cat = &i->value.value;
			
				if (cat->count > 0)
				{	
					const char *name = xed_iclass_enum_t2str((xed_iclass_enum_t)i->value.key);
					for (const char *ptr = name; *ptr; ptr++)
						fprintf(fptr, "%c", tolower(*ptr));
					fprintf(fptr, "\t%s\t%lu\t%lu\t%lu\t%f\t", cat_names[category], cat->count, cat->mem_count, cat->count - cat->mem_count, (float)cat->count/(float)total);
					for (int j = 0; j < col; j++)
						fprintf(fptr, "\t\t");
					fprintf(fptr, "%f\t%f\n",
							(float)(cat->count - cat->mem_count)/(float)total,
							(float)cat->mem_count/(float)total);
				}
			}
			fclose(fptr);
		}

		{
			uint64_t int_sums[opcodes.size()] = {0};
			uint64_t sig_sums[opcodes.size()] = {0};
			uint64_t exp_sums[opcodes.size()] = {0};
			for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
			{
				for (int j = 0; j < 65; j++)
					int_sums[i.idx()] += i->value.value.int_bitwidth[j];
				for (int j = 0; j < 53; j++)
					sig_sums[i.idx()] += i->value.value.sig_lbitwidth[j];
				for (int j = 0; j < 17; j++)
					exp_sums[i.idx()] += i->value.value.exp_bitwidth[j];
			}

			{
				fptr = fopen("integer_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
				{
					if (int_sums[i.idx()] > 0)
					{
						const char *name = xed_iclass_enum_t2str((xed_iclass_enum_t)i->value.key);
						for (const char *ptr = name; *ptr; ptr++)
							fprintf(fptr, "%c", tolower(*ptr));
						fprintf(fptr, "\tProbability\tCumulative\t");
					}
				}
				fprintf(fptr, "\n");

				uint64_t cumulative[opcodes.size()] = {0};
				for (int j = 0; j < 65; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
					{
						opcode_t *cat = &i->value.value;

						if (int_sums[i.idx()] > 0)
						{
							cumulative[i.idx()] += cat->int_bitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", cat->int_bitwidth[j], (float)cat->int_bitwidth[j]/(float)int_sums[i.idx()], (float)cumulative[i.idx()]/(float)int_sums[i.idx()]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}

			{
				fptr = fopen("sig_lsbf_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
				{
					if (sig_sums[i.idx()] > 0)
					{
						const char *name = xed_iclass_enum_t2str((xed_iclass_enum_t)i->value.key);
						for (const char *ptr = name; *ptr; ptr++)
							fprintf(fptr, "%c", tolower(*ptr));
						fprintf(fptr, "\tProbability\tCumulative\t");
					}
				}
				fprintf(fptr, "\n");

				uint64_t cumulative[opcodes.size()] = {0};
				for (int j = 0; j < 53; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
					{
						opcode_t *cat = &i->value.value;

						if (sig_sums[i.idx()] > 0)
						{
							cumulative[i.idx()] += cat->sig_lbitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", cat->sig_lbitwidth[j], (float)cat->sig_lbitwidth[j]/(float)sig_sums[i.idx()], (float)cumulative[i.idx()]/(float)sig_sums[i.idx()]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}

			{
				fptr = fopen("sig_msbf_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
				{
					if (sig_sums[i.idx()] > 0)
					{
						const char *name = xed_iclass_enum_t2str((xed_iclass_enum_t)i->value.key);
						for (const char *ptr = name; *ptr; ptr++)
							fprintf(fptr, "%c", tolower(*ptr));
						fprintf(fptr, "\tProbability\tCumulative\t");
					}
				}
				fprintf(fptr, "\n");

				uint64_t cumulative[opcodes.size()] = {0};
				for (int j = 0; j < 53; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
					{
						opcode_t *cat = &i->value.value;

						if (sig_sums[i.idx()] > 0)
						{
							cumulative[i.idx()] += cat->sig_mbitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", cat->sig_mbitwidth[j], (float)cat->sig_mbitwidth[j]/(float)sig_sums[i.idx()], (float)cumulative[i.idx()]/(float)sig_sums[i.idx()]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}

			{
				fptr = fopen("exp_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
				{
					if (exp_sums[i.idx()] > 0)
					{
						const char *name = xed_iclass_enum_t2str((xed_iclass_enum_t)i->value.key);
						for (const char *ptr = name; *ptr; ptr++)
							fprintf(fptr, "%c", tolower(*ptr));
						fprintf(fptr, "\tProbability\tCumulative\t");
					}
				}
				fprintf(fptr, "\n");

				uint64_t cumulative[opcodes.size()] = {0};
				for (int j = 0; j < 17; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (core::map<uint64_t, implier<uint64_t, opcode_t> >::iterator i = opcodes.rbegin(); i != opcodes.rend(); i--)
					{
						opcode_t *cat = &i->value.value;

						if (exp_sums[i.idx()] > 0)
						{
							cumulative[i.idx()] += cat->exp_bitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", cat->exp_bitwidth[j], (float)cat->exp_bitwidth[j]/(float)exp_sums[i.idx()], (float)cumulative[i.idx()]/(float)exp_sums[i.idx()]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}
		}

		{
			uint64_t int_sums[6] = {0};
			uint64_t sig_sums[6] = {0};
			uint64_t exp_sums[6] = {0};
			for (int i = 0; i < 6; i++)
			{
				for (int j = 0; j < 65; j++)
					int_sums[i] += categories[i].int_bitwidth[j];
				for (int j = 0; j < 53; j++)
					sig_sums[i] += categories[i].sig_lbitwidth[j];
				for (int j = 0; j < 17; j++)
					exp_sums[i] += categories[i].exp_bitwidth[j];
			}

			{
				fptr = fopen("category_integer_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (int i = 0; i < 6; i++)
					if (int_sums[i] > 0)
						fprintf(fptr, "%s\tProbability\tCumulative\t", cat_names[i]);
				fprintf(fptr, "\n");

				uint64_t cumulative[6] = {0};
				for (int j = 0; j < 65; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (int i = 0; i < 6; i++)
					{
						if (int_sums[i] > 0)
						{
							cumulative[i] += categories[i].int_bitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", categories[i].int_bitwidth[j], (float)categories[i].int_bitwidth[j]/(float)int_sums[i], (float)cumulative[i]/(float)int_sums[i]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}

			{
				fptr = fopen("category_sig_lsbf_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (int i = 0; i < 6; i++)
					if (sig_sums[i] > 0)
						fprintf(fptr, "%s\tProbability\tCumulative\t", cat_names[i]);
				fprintf(fptr, "\n");

				uint64_t cumulative[6] = {0};
				for (int j = 0; j < 53; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (int i = 0; i < 6; i++)
					{
						if (sig_sums[i] > 0)
						{
							cumulative[i] += categories[i].sig_lbitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", categories[i].sig_lbitwidth[j], (float)categories[i].sig_lbitwidth[j]/(float)sig_sums[i], (float)cumulative[i]/(float)sig_sums[i]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}

			{
				fptr = fopen("category_sig_msbf_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (int i = 0; i < 6; i++)
					if (sig_sums[i] > 0)
						fprintf(fptr, "%s\tProbability\tCumulative\t", cat_names[i]);
				fprintf(fptr, "\n");

				uint64_t cumulative[6] = {0};
				for (int j = 0; j < 53; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (int i = 0; i < 6; i++)
					{
						if (sig_sums[i] > 0)
						{
							cumulative[i] += categories[i].sig_mbitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", categories[i].sig_mbitwidth[j], (float)categories[i].sig_mbitwidth[j]/(float)sig_sums[i], (float)cumulative[i]/(float)sig_sums[i]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}

			{
				fptr = fopen("category_exp_bitwidth.tsv", "w");
				fprintf(fptr, "Bitwidth\t");
				for (int i = 0; i < 6; i++)
					if (exp_sums[i] > 0)
						fprintf(fptr, "%s\tProbability\tCumulative\t", cat_names[i]);
				fprintf(fptr, "\n");

				uint64_t cumulative[6] = {0};
				for (int j = 0; j < 17; j++)
				{
					fprintf(fptr, "%d\t", j);
					for (int i = 0; i < 6; i++)
					{
						if (exp_sums[i] > 0)
						{
							cumulative[i] += categories[i].exp_bitwidth[j];
							fprintf(fptr, "%lu\t%f\t%f\t", categories[i].exp_bitwidth[j], (float)categories[i].exp_bitwidth[j]/(float)exp_sums[i], (float)cumulative[i]/(float)exp_sums[i]);
						}
					}
					fprintf(fptr, "\n");
				}
				fclose(fptr);	
			}
		}
	}

	{
		file::table<core::implier<uint64_t, uint64_t> > instr("instrs.tbl", false);
	}
}

