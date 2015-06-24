#include <windows.h>
#include "tier1/strtools.h"
#include "BitBuffer.h"
#include "mem.h"

BitBuffer::BitBuffer(void)
{
	sizeError = false;
	data = NULL;
	currentByte = NULL;
	currentBit = 0;
	maxSize = 0;
	littleEndian = false;
	ownData = false;
}

BitBuffer::~BitBuffer(void)
{
	Free();
}

void BitBuffer::Clear(void)
{
	Q_memset(data, 0, maxSize);

	Reset();
}

void BitBuffer::Reset(void)
{
	currentBit = 0;
	sizeError = false;
	littleEndian = true;
	currentByte = data;
}

bool BitBuffer::Resize(unsigned int size)
{
	Free();

	data = (unsigned char *)Mem_ZeroMalloc(size + 4);

	if (!data)
	{
		maxSize = 0;
		currentByte = NULL;
		currentBit = 0;
		sizeError = false;
		ownData = false;
		return false;
	}

	currentByte = data;
	maxSize = size;
	currentBit = 0;
	sizeError = false;
	ownData = true;
	littleEndian = true;
	return true;
}

void BitBuffer::Free(void)
{
	if (data && ownData)
		Mem_Free(data);

	maxSize = 0;
	data = NULL;
	currentByte = NULL;
	currentBit = 0;
	ownData = false;
	sizeError = false;
	littleEndian = true;
}

unsigned int BitBuffer::CurrentBit(void)
{
	return currentBit + ((currentByte - data) * 8);
}