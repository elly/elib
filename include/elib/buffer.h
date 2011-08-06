/* buffer.h */

#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

typedef struct buffer buffer;

buffer *buffer_new(void);
buffer *buffer_newfrom(const void *data, size_t size);

int buffer_append(buffer *b, const void *data, size_t size);
int buffer_appends(buffer *b, const char *str);

size_t buffer_size(const buffer *b);
void *buffer_data(const buffer *b);
void buffer_free(buffer *b);

#endif /* !BUFFER_H */
