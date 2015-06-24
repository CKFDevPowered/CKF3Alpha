#ifndef CRC_H
typedef struct
{
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
}
MD5Context_t;
#endif

void MD5Init(MD5Context_t *ctx);
void MD5Update(MD5Context_t *ctx, unsigned char const *buf, unsigned int len);
void MD5Final(unsigned char digest[16], MD5Context_t *ctx);
void MD5Transform(unsigned int buf[4], unsigned int const in[16]);
int MD5_Hash_File(unsigned __int8 *digest, char *pszFileName, int bSeed = FALSE, unsigned int *seed = NULL);