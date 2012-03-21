/* elib/uuid.h */

#ifndef ELIB_UUID_H
#define ELIB_UUID_H

#define UUIDLEN		16
#define UUIDSTRSIZE	37

/* Generates a new uuid into the buffer pointed to by |uuid|, which must be of
 * at least UUIDLEN bytes. */
void uuidgen(unsigned char *uuid);
/* Converts |uuid| into a string, storing it at |str|, which must point to at
 * least UUIDSTRSIZE bytes of free space. Null-terminates. */
void uuid2str(char *str, const unsigned char *uuid);
/* Converts |str| into a uuid, storing it at |uuid|, which must point to at
 * least UUIDLEN bytes of free space. */
void str2uuid(unsigned char *uuid, const char *str);
/* Returns the string form of |uuid|. Uses a static buffer; hence not reentrant.
 * Use with caution. */
const char *uuidstr(const unsigned char *uuid);

#endif /* !ELIB_UUID_H */
