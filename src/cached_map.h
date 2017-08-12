#pragma once

#include "map.h"
#include "table.h"

#include <stdlib.h>
#include <stdio.h>

template <typename key_type, typename value_type, int limit, value_type (*U)(value_type, value_type)>
struct cached_map
{
	cached_map() {}
	cached_map(const char *filename, bool overwrite = true) : store(filename, overwrite) { this->filename = filename; }
	~cached_map()
	{
		save();
	}

	typedef core::map<key_type, value_type> cache_type;
	typedef file::map<key_type, value_type> store_type;
	const char *filename;

	store_type store;
	cache_type cache;

	value_type *get(const key_type &key)
	{
		if (cache.size() >= limit)
			save();
		typename cache_type::iterator loc = cache.at(key);
		return &loc->value;
	}	

	void update(const key_type &key, const value_type &value)
	{
		cache.update(key, value, U);
		
		if (cache.size() >= limit)
			save();
	}

	void save()
	{
		printf("\nSaving %d items into %s[%d]...\t\t", cache.size(), filename, store.size());
		fflush(stdout);
		store.append(cache, U);
		printf("Done %d[%d]\n", cache.size(), store.size());
	}
};

