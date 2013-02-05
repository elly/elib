#include <elib/chunkfile.h>
#include <stdio.h>

static int print_ihdr(struct chunkfile *cf, struct chunkfile_chunk *c)
{
	uint8_t *buf = malloc(c->length);
	int i;
	chunkfile_readchunk(cf, c, buf, c->length, 0);
	printf("IHDR ");
	for (i = 0; i < c->length; i++)
		printf("%02x ",  buf[i]);
	printf("\n");
}

static int print_chunk(struct chunkfile *cf, struct chunkfile_chunk *c)
{
	printf("chunk: %s\n", c->type);
	return 0;
}

int main(int argc, char *argv[])
{
	const uint8_t magic[] = { 0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a };
	struct chunkfile *cf = chunkfile_new();
	chunkfile_set_magic(cf, magic, sizeof(magic));
	chunkfile_hook(cf, "IHDR", print_ihdr);
	chunkfile_hook_unrec_mand(cf, print_chunk);
	chunkfile_hook_unrec_opt(cf, print_chunk);
	chunkfile_parse(cf, fopen(argv[1], "rb"));
	return 0;
}
