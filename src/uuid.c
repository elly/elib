/* uuid.c */

#include <elib/uuid.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void uuidgen(unsigned char *uuid) {
	static int rndfd = -1;
	if (rndfd == -1)
		rndfd = open("/dev/urandom", O_RDONLY);
	if (rndfd == -1)
		abort();
	read(rndfd, uuid, UUIDLEN);
}

void uuid2str(char *str, const unsigned char *uuid) {
	static const char *hex = "0123456789abcdef";
	int i;
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
