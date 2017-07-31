#pragma once

template <class entry>
struct table
{
	table()
	{
		fptr = NULL;
	}

	~table()
	{
		close();
	}

	FILE *fptr;

	bool open(char *filename, int length)
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

	void write(int index, const entry &e)
	{
		fseek(fptr, index*sizeof(entry), SEEK_SET);
		fwrite(&result, sizeof(entry), 1, fptr);
	}
};
