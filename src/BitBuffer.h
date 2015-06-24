#ifndef BITBUFFER_H
#define BITBUFFER_H

class BitBuffer
{
public:
	BitBuffer(void);
	~BitBuffer(void);

public:
	void Clear(void);
	void FastClear(void);
	void Reset(void);
	bool Resize(unsigned int size);
	void Free(void);
	int SetBuffer(unsigned char *buffer, int size);
	void ConcatBuffer(BitBuffer *buffer);
	unsigned int CurrentBit(void);
	int CurrentSize(void);

	unsigned int SpaceLeft(void);
	void AlignByte(void);
	void SkipBytes(int n);
	int SkipBits(int n);
	int SkipString(void);

	void StartBitMode(void);
	void EndBitMode(void);

	int ReadBit(void);
	unsigned int ReadBits(int n);
	int ReadSBits(int numbits);
	unsigned int PeekBits(int numbits);
	int ReadChar(void);
	int ReadByte(void);
	int ReadShort(void);
	int ReadWord(void);
	unsigned int ReadLong(void);
	float ReadFloat(void);
	bool ReadBuf(int iSize, void *pbuf);
	char *ReadString(void);
	char *ReadStringLine(void);
	float ReadAngle(void);
	float ReadHiresAngle(void);
	double ReadBitAngle(signed int numbits);
	int ReadBitString(void);
	int ReadBitData(unsigned char *p, int length);
	int ReadBitVec3Coord(int fa);
	double ReadBitCoord(void);
	double ReadCoord(void);

	void WriteBit(int c);
	void WriteBits(unsigned int bits, int n);
	void WriteBuf(BitBuffer *buf, int length);
	void WriteSBits(int bits, int numbits);
	void WriteChar(int c);
	void WriteByte(int c);
	void WriteShort(int c);
	void WriteWord(int c);
	void WriteLong(unsigned int c);
	void WriteFloat(float f);
	void WriteString(const char *s);
	void WriteBuf(const void *buf, int iSize);
	void WriteBitData(unsigned char *p, int length);
	void WriteAngle(float f);
	void WriteHiresAngle(float f);
	void WriteBitAngle(float fAngle, int numbits);
	void WriteBitString(char *pch);
	void WriteCoord(float f);

private:
	bool sizeError;
	unsigned char *data;
	unsigned char *currentByte;
	int currentBit;
	int maxSize;
	bool littleEndian;
	bool ownData;
};

#endif