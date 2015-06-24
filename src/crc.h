#ifndef CRC_H
#define CRC_H
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

typedef struct
{
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
}
MD5Context_t;

typedef unsigned long CRC32_t;

typedef struct crc_info_s
{
	qboolean needcheck;
	qboolean loaded;
	CRC32_t crc;
}
crc_info_t;

void CRC32_Init(CRC32_t *pulCRC);
CRC32_t CRC32_Final(CRC32_t pulCRC);
void CRC32_ProcessBuffer(CRC32_t *pulCRC, void *p, int len);
void CRC32_ProcessByte(CRC32_t *pulCRC, unsigned char ch);
int CRC_File(CRC32_t *crcvalue, char *pszFileName);
unsigned char COM_BlockSequenceCRCByte(unsigned char *base, int length, int sequence);

void MD5Init(MD5Context_t *context);
void MD5Update(MD5Context_t *context, unsigned char const *buf, unsigned int len);
void MD5Final(unsigned char digest[16], MD5Context_t *context);
void Transform(unsigned int buf[4], unsigned int const in[16]);
int MD5_Hash_File(unsigned char digest[16], char *pszFileName, int bUsefopen, int bSeed, unsigned int seed[4]);
char *MD5_Print(unsigned char hash[16]);
int MD5_Hash_CachedFile(unsigned char digest[16], unsigned char *pCache, int nFileSize, int bSeed, unsigned int seed[4]);
int CRC_MapFile(CRC32_t *crcvalue, char *pszFileName);

#endif