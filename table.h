#pragma once

template <class entry>
struct table
{
	table()
	{
		fptr = NULL;
	}

	table(char *filename)
	{
		fptr = NULL;
		open(filename);
	}

	~table()
	{
		close();
	}

	FILE *fptr;

	bool open(char *filename)
	{
		if (fptr != NULL)
			return false;

		fptr = fopen(filename, "r+b");
		if (fptr == NULL)
			fptr = fopen(filename, "w+b");
		if (fptr == NULL)
			return false;
		else
			return true;
	}

	void close()
	{
		if (fptr != NULL)
			fclose(fptr);
		fptr = NULL;
	}

	entry read(int index)
	{
		entry result;
		fseek(fptr, index*sizeof(entry), SEEK_SET);
		fread(&result, sizeof(entry), 1, fptr);
		return result;
	}

	void insert(int index, const entry &e)
	{
		entry r[2];
		int i = 0;

		r[1-i] = e;

		fseek(fptr, index*sizeof(entry), SEEK_SET);
		while (fread(r+i, sizeof(entry), 1, fptr) > 0)
		{
			fseek(fptr, -sizeof(entry), SEEK_CUR);
			fwrite(r+(1-i), sizeof(entry), 1, fptr);
			i = 1-i;
		}
		fwrite(r+(1-i), sizeof(entry, 1, fptr);
	}

	void write(int index, const entry &e)
	{
		fseek(fptr, index*sizeof(entry), SEEK_SET);
		fwrite(&result, sizeof(entry), 1, fptr);
	}
};
