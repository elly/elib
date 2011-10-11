/* elib/uuid.h */

#ifndef ELIB_UUID_H
#define ELIB_UUID_H

#define UUIDLEN		16
#define UUIDSTRSIZE	37

void uuidgen(unsigned char *uuid);
void uuid2str(char *str, const unsigned char *uuid);
void str2uuid(unsigned char *uuid, const char *str);
const char *uuidstr(const unsigned char *uuid);

#endif /* !ELIB_UUID_H */
