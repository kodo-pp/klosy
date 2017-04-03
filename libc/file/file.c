#include <file.h>
#include <time.h>
#include <hash.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/memory.h>
static const size_t DEFAULT_FILE_SIZE = 16;
static int frw_err = 0;

int get_frw_err()
{
	return frw_err;
}
void reset_frw_err()
{
	frw_err = 0;
}

static inline int try_resize(file f)
{
	if (f == null)
		return 0;
	else
		return resize_file(f, (f->size > 0) ? 2*f->size : DEFAULT_FILE_SIZE);
}

file make_file(string_t name)
{
	file f = allocate(sizeof(struct _file));
	if (f == null)
		return null;
	uint64_t h = hash(name);
	f->name_hash = h;
	f->size = DEFAULT_FILE_SIZE;
	f->data = allocate(DEFAULT_FILE_SIZE);
	f->pos = 0;
	if (f->data == null)
	{
		deallocate(f, sizeof(struct _file));
		return null;
	}
//	printf("make file: name=[%s],size=%d,pos=%d;\n", name, (int)f->size, (int)f->pos);
//	sleep(1000);
	return f;
}

int resize_file(file f, size_t new_size)
{
	if (f == null)
		return 0;
	if (new_size < f->size)
	{
		deallocate(f->data + new_size, f->size - new_size);
		f->size = new_size;
	}
	else if (new_size > f->size)
	{
		void *new_data = allocate(new_size);
		if (new_data == null)
			return 0;
		memset(new_data, 0, new_size);
		memcpy(new_data, f->data, f->size);
		deallocate(f->data, f->size);
		f->size = new_size;
		f->data = new_data;
	}
	return 1;
}

int rename_file(file f, string_t new_name)
{
	if (f == null)
		return 0;
	int64_t nh = hash(new_name);
	f->name_hash = nh;
	return 1; 
}

int8_t fread_8(file f)
{
	if (f == null)
	{
		frw_err = 1;
		return 0;
	}
	else if (f->pos >= f->size)
	{
		frw_err = 2;
		return 0;
	}
	else
	{
		int8_t *ptr = (int8_t *)(f->data);
		int8_t ret = ptr[f->pos++];
		return ret;
	}
}

int fread(file f, size_t sz, void *_buf)
{
	if (f == null || _buf == null)
		return 0;
	int8_t *buf = (int8_t *)_buf;
	for (size_t i = 0; i < sz; ++i)
	{
		buf[i] = fread_8(f);
		if (get_frw_err())
		{
			return 0;
		}
	}
	return 1;
}

int fwrite_8(file f, int8_t data)
{
//	printf("\nfw8 %d: ", (int)data);
//	sleep(1000);
	if (f == null)
		return 0;
	if (f->pos >= f->size)
	{
		if (!try_resize(f))
			return 0;
	}
	int8_t *p = (int8_t *)(f->data);
	p[f->pos++] = data;
	return 1;
}
int fwrite(file f, size_t sz, void *_buf)
{
	if (f == null || _buf == null)
	{
		return 0;
	}
	int8_t *buf = (int8_t *)_buf;
//	printf("buf ready, ");
//	sleep(1000);
	for (size_t i = 0; i < sz; ++i)
	{
//		printf("fwr %d ", (int)buf[i]);
//		sleep(1000);
		if (!fwrite_8(f, buf[i]))
			return 0;
//		printf("ok, ");
//		sleep(1000);
	}
//	printf("ok\n");
	return 1;
}

