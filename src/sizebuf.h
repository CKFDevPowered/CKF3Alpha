typedef struct sizebuf_s
{
	char *buffername;
	int flags;
	byte *data;
	int maxsize;
	int cursize;
}
sizebuf_t;

#define FSB_ALLOWOVERFLOW (1<<0)
#define FSB_OVERFLOWED (1<<1)