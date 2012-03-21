/* buffer.h - growable bytebuffers */

#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

typedef struct buffer buffer;

/* Allocate a new empty buffer. */
buffer *buffer_new(void);
/* Allocate a new buffer, filled with |size| bytes from |data|. */
buffer *buffer_newfrom(const void *data, size_t size);
/* Frees |b|. */
void buffer_free(buffer *b);

/* Append |size| bytes from |data| to |b|. */
int buffer_append(buffer *b, const void *data, size_t size);
/* Append the contents of |str| to |b|, not including the trailing \0. */
int buffer_appends(buffer *b, const char *str);

/* Returns the amount of data in |b|. */
size_t buffer_size(const buffer *b);
/* Returns a pointer to the data in |b|. You may write through this pointer, but
 * only up to buffer_size(b), or you will corrupt the buffer. */
void *buffer_data(const buffer *b);

#endif /* !BUFFER_H */
