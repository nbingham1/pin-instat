#pragma once

#include "table.h"
#include "array.h"

#include <stdlib.h>
#include <stdio.h>

template <typename value_type, int limit>
struct cached_table
{
	cached_table()
	{
		for (int i = 0; i < limit; i++)
			cache[i].key = (uint64_t)-1;
		count = 0;
	}

	cached_table(const char *filename, bool overwrite = true) : store(filename, overwrite)
	{
		this->filename = filename;
		for (int i = 0; i < limit; i++)
			cache[i].key = (uint64_t)-1;
		count = 0;
	}

	~cached_table()
	{
		finish();
		store.close();
	}

	typedef implier<uint64_t, value_type> cache_type;
	typedef file::table<value_type> store_type;
	const char *filename;

	store_type store;
	cache_type cache[limit];
	int count;

	void finish(FILE *fptr = stdout)
	{
		fprintf(fptr, "Saving %d values to %s[%d]...\t\t", count, filename, store.size());
		if (store.fptr != NULL)
		{
			for (int i = 0; i < limit; i++)
				if (cache[i].key != (uint64_t)-1)
				{
					store.write(cache[i].key, cache[i].value);
					cache[i].key = (uint64_t)-1;
					count--;
				}
		}
		fprintf(fptr, "Done %d[%d]\n", count, store.size());
	}

	value_type *get(uint64_t index, FILE *log = stdout)
	{
		cache_type *loc = cache + (index%limit);
		if (loc->key != index) {
			if (loc->key != (uint64_t)-1) {
				fprintf(log, "Saving 1 value to %s[%d]...\t\t", filename, store.size());
				store.write(loc->key, loc->value);
				count--;
				fprintf(log, "Done [%d]\n", store.size());
			}
			loc->key = index;
			loc->value = store.read(loc->key);
			count++;
		}
		return &loc->value;
	}

	value_type read(uint64_t index)
	{
		cache_type *loc = cache + (index%limit);
		if (loc->key != index) {
			if (loc->key != (uint64_t)-1) {
				store.write(loc->key, loc->value);
				count--;
			}
			loc->key = index;
			loc->value = store.read(loc->key);
			count++;
		}
		return loc->value;
	}

	void write(uint64_t index, const value_type &v)
	{
		cache_type *loc = cache + (index%limit);
		if (loc->key != (uint64_t)-1 && loc->key != index) {
			store.write(loc->key, loc->value);
		}
		loc->key = index;
		loc->value = v;
	}
};

