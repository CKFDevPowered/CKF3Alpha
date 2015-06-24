#pragma once

struct TIceSubKey;

class CIceKey
{
public:
	CIceKey(INT n = 0);
	~CIceKey(void);

	VOID SetKey(CONST BYTE *pKey);

	VOID Encrypt(CONST BYTE pPlainText[8], BYTE pCipherText[8]) const;
	VOID Decrypt(CONST BYTE pCipherText[8], BYTE pPlainText[8]) const;

	INT GetKeySize() const { return m_iSize << 3; }
	static INT GetBlockSize() { return 8; }

private:
	VOID BuildSchedule(USHORT *usKeyBuilder, INT n, CONST INT *cpiKeyRotation);

	INT m_iSize;
	INT m_iRounds;
	TIceSubKey *m_pKeySchedule;
};