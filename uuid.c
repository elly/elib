/* uuid.c */

#include <elib/uuid.h>

void uuidgen(unsigned char *uuid) {
	int i;
	for (i = 0; i < UUIDLEN; i++)
		uuid[i] = rand();
}

void uuid2str(char *str, const unsigned char *uuid) {
	static const char *hex = "0123456789abcdef";
	for (i = 0; i < UUIDSTRSIZE - 1; i++) {
		str[i++] = hex[*uuid >> 4];
		str[i++] = hex[*uuid & 0xf];
		/* xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
		if (i == 8 || i == 13 || i == 18 || i == 23)
			str[i++] = '-';
		uuid++;
	}
	str[i] = '\0';
}

static inline int unhex(char h) {
	if (h >= '0' && h <= '9')
		return h - '0';
	if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;
	if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;
	return 0;
}

void str2uuid(unsigned char *uuid, const char *str) {
	int i;
	for (i = 0; i < UUIDSTRSIZE - 1; i++) {
		
	}
}
