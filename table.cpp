#include "table.h"

table::table()
{
	fptr = NULL;
	format = NULL;
	offset = NULL;
	elements = 0;

	buffer = NULL
}

table::~table()
{
	close();
}

int table::size(TYPE t)
{
	switch (t)
	{
	case BOOL:
	case CHAR:
	case UCHAR:
	case INT8:
	case UINT8:
		return 1;
	case INT16:
	case UINT16:
		return 2;
	case INT32:
	case UINT32:
	case FLOAT:
		return 4;
	case INT64:
	case UINT64:
	case DOUBLE:
		return 8;
	default:
		return 0;
	}
}

bool table::open(char *filename, int count, ...)
{
	if (fptr != NULL)
		return false;

	fptr = fopen(filename, "r+b");
	if (fptr == NULL)
		fptr = fopen(filename, "w+b");
	if (fptr == NULL)
		return false;

	format = new unsigned char[count];
	offset = new int[count+1];
	elements = count;

	va_list args;
	va_start(args, count);
	
	offset[0] = 0;
	for (int i = 0; i < count; i++) {
		format[i] = va_arg(args, TYPE);
		offset[i+1] = offset[i] + size(format[i]);
	}
	va_end(args);

	buffer = new unsigned char[offset[elements]];

	return true;
}

void table::close()
{
	if (fptr != NULL)
		fclose(fptr);
	fptr = NULL;

	if (format != NULL)
		delete [] format;
	format = NULL;

	if (offset != NULL)
		delete [] offset;
	offset = NULL;

	elements = 0;

	if (buffer != NULL)
		delete [] buffer;
	buffer = NULL;
}

void table::reset()
{
	memset(buffer, 0, offset[elements]);
}

void table::read(int entry)
{
	int length = offset[elements];
	fseek(fptr, length*entry, SEEK_SET);
	if (fread(buffer, 1, length, fptr) != length)
		memset(buffer, 0, length);
}

void table::write(int entry)
{
	int length = offset[elements];
	fseek(fptr, length*entry, SEEK_SET);
	fwrite(buffer, 1, length, fptr);
}

void table::reset()
{
	memset(buffer, 0, offset[elements]);
}

void table::get(int count, ...)
{	
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; i++)
	{
		int j = va_arg(args, int);
		switch (format[j])
		{
		case BOOL:   *va_arg(args, bool*) = *((bool*)(buffer + offset[j])); break;
		case CHAR:   *va_arg(args, char*) = *((char*)(buffer + offset[j])); break;
		case UCHAR:  *va_arg(args, unsigned char*) = *((unsigned char*)(buffer + offset[j])); break;
		case INT8:   *va_arg(args, int8_t*) = *((int8_t*)(buffer + offset[j])); break;
		case UINT8:  *va_arg(args, uint8_t*) = *((uint8_t*)(buffer + offset[j])); break;
		case INT16:  *va_arg(args, int16_t*) = *((int16_t*)(buffer + offset[j])); break;
		case UINT16: *va_arg(args, uint16_t*) = *((uint16_t*)(buffer + offset[j])); break;
		case INT32:  *va_arg(args, int32_t*) = *((int32_t*)(buffer + offset[j])); break;
		case UINT32: *va_arg(args, uint32_t*) = *((uint32_t*)(buffer + offset[j])); break;
		case INT64:  *va_arg(args, int64_t*) = *((int64_t*)(buffer + offset[j])); break;
		case UINT64: *va_arg(args, uint64_t*) = *((uint64_t*)(buffer + offset[j])); break;
		case FLOAT:  *va_arg(args, float*) = *((float*)(buffer + offset[j])); break;
		case DOUBLE: *va_arg(args, double*) = *((double*)(buffer + offset[j])); break;
		default: break;
		}
	}
	va_end(args);
}

void table::set(int count, ...)
{	
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; i++)
	{
		int j = va_arg(args, int);
		switch (format[j])
		{
		case BOOL:   *((bool*)(buffer + offset[j])) = va_arg(args, bool); break;
		case CHAR:   *((char*)(buffer + offset[j])) = va_arg(args, char); break;
		case UCHAR:  *((unsigned char*)(buffer + offset[j])) = va_arg(args, unsigned char); break;
		case INT8:   *((int8_t*)(buffer + offset[j])) = va_arg(args, int8_t); break;
		case UINT8:  *((uint8_t*)(buffer + offset[j])) = va_arg(args, uint8_t); break;
		case INT16:  *((int16_t*)(buffer + offset[j])) = va_arg(args, int16_t); break;
		case UINT16: *((uint16_t*)(buffer + offset[j])) = va_arg(args, uint16_t); break;
		case INT32:  *((int32_t*)(buffer + offset[j])) = va_arg(args, int32_t); break;
		case UINT32: *((uint32_t*)(buffer + offset[j])) = va_arg(args, uint32_t); break;
		case INT64:  *((int64_t*)(buffer + offset[j])) = va_arg(args, int64_t); break;
		case UINT64: *((uint64_t*)(buffer + offset[j])) = va_arg(args, uint64_t); break;
		case FLOAT:  *((float*)(buffer + offset[j])) = va_arg(args, float); break;
		case DOUBLE: *((double*)(buffer + offset[j])) = va_arg(args, double); break;
		default: break;
		}
	}
	va_end(args);
}

void table::add(int count, ...)
{	
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; i++)
	{
		int j = va_arg(args, int);
		switch (format[j])
		{
		case BOOL:   *((bool*)(buffer + offset[j])) += va_arg(args, bool); break;
		case CHAR:   *((char*)(buffer + offset[j])) += va_arg(args, char); break;
		case UCHAR:  *((unsigned char*)(buffer + offset[j])) += va_arg(args, unsigned char); break;
		case INT8:   *((int8_t*)(buffer + offset[j])) += va_arg(args, int8_t); break;
		case UINT8:  *((uint8_t*)(buffer + offset[j])) += va_arg(args, uint8_t); break;
		case INT16:  *((int16_t*)(buffer + offset[j])) += va_arg(args, int16_t); break;
		case UINT16: *((uint16_t*)(buffer + offset[j])) += va_arg(args, uint16_t); break;
		case INT32:  *((int32_t*)(buffer + offset[j])) += va_arg(args, int32_t); break;
		case UINT32: *((uint32_t*)(buffer + offset[j])) += va_arg(args, uint32_t); break;
		case INT64:  *((int64_t*)(buffer + offset[j])) += va_arg(args, int64_t); break;
		case UINT64: *((uint64_t*)(buffer + offset[j])) += va_arg(args, uint64_t); break;
		case FLOAT:  *((float*)(buffer + offset[j])) += va_arg(args, float); break;
		case DOUBLE: *((double*)(buffer + offset[j])) += va_arg(args, double); break;
		default: break;
		}
	}
	va_end(args);
}

