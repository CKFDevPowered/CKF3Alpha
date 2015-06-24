#ifndef PARSEMSG_H
#define PARSEMSG_H

#define ASSERT(x)

void BEGIN_READ(void *buf, int size);
int READ_CHAR(void);
int READ_BYTE(void);
int READ_SHORT(void);
int READ_WORD(void);
int READ_LONG(void);
float READ_FLOAT(void);
char *READ_STRING(void);
float READ_COORD(void);
float READ_ANGLE(void);
float READ_HIRESANGLE(void);
int READ_OK(void);

class BufferWriter
{
public:
	BufferWriter(void);
	BufferWriter(unsigned char *buffer, int bufferLen);

public:
	void Init(unsigned char *buffer, int bufferLen);

	void WriteByte(unsigned char data);
	void WriteLong(int data);
	void WriteString(const char *str);

	bool HasOverflowed(void);
	int GetSpaceUsed(void);

protected:
	unsigned char *m_buffer;
	int m_remaining;
	bool m_overflow;
	int m_overallLength;
};

#endif