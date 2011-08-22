#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <elib/buffer.h>
#include <elib/util.h>

struct buffer {
	size_t size;
	void *data;
};

struct buffer *buffer_new(void) {
	struct buffer *b = emalloc(sizeof *b);
	if (!b)
		return NULL;
	b->size = 0;
	b->data = NULL;
	return b;
}

struct buffer *buffer_newfrom(const void *data, size_t size) {
	struct buffer *b = emalloc(sizeof *b);
	if (!b)
		return NULL;
	b->size = size;
	b->data = emalloc(size);
	if (!b->data) {
		efree(b, sizeof *b);
		return NULL;
	}
	memcpy(b->data, data, size);
	return b;
}

int buffer_append(buffer *b, const void *data, size_t size) {
	void *n = erealloc(b->data, b->size + size);
	if (!n)
		return -ENOMEM;
	b->data = n;
	memcpy(b->data + b->size, data, size);
	b->size += size;
	return 0;
}

int buffer_appends(buffer *b, const char *str) {
	return buffer_append(b, str, strlen(str));
}

size_t buffer_size(const struct buffer *b) {
	return b->size;
}

void *buffer_data(const struct buffer *b) {
	return b->data;
}

void buffer_free(struct buffer *b) {
	efree(b->data, b->size);
	efree(b, sizeof *b);
}
