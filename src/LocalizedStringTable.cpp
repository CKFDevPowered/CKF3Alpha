#pragma warning(disable: 4018)

#if !defined (_X360)
#include <windows.h>
#endif
#include <wchar.h>

#include "FileSystem.h"
#include "KeyValues.h"

#include "vgui_internal.h"
#include "vgui/ILocalize.h"
#include "vgui/ISystem.h"
#include "vgui/ISurface.h"

#include "tier1/UtlVector.h"
#include "tier1/UtlRBTree.h"
#include "tier1/UtlSymbol.h"
#include "tier1/UtlString.h"
#include "UnicodeFileHelpers.h"
#include "tier0/icommandline.h"
#include "byteswap.h"
#include "configs.h"

#if defined (_X360)
#include "xbox/xbox_win32stubs.h"
#endif

#include <tier0/memdbgon.h>

using namespace vgui;

#define MAX_LOCALIZED_CHARS 4096

class CLocalizedStringTable : public vgui::ILocalize
{
public:
	CLocalizedStringTable(void);
	~CLocalizedStringTable(void);

public:
	bool AddFile(IFileSystem *fileSystem, const char *szFileName);

	bool SaveToFile(IFileSystem *fileSystem, const char *fileName);
	bool SaveToFile(const char *fileName);

	void AddString(const char *tokenName, wchar_t *unicodeString, const char *fileName);

	wchar_t *Find(const char *pName);
	StringIndex_t FindIndex(const char *pName);

	void RemoveAll(void);

	int ConvertANSIToUnicode(const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes);
	int ConvertUnicodeToANSI(const wchar_t *unicode, char *ansi, int ansiBufferSize);

	void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, ...);

	StringIndex_t GetFirstStringIndex(void);
	StringIndex_t GetNextStringIndex(StringIndex_t index);

	const char *GetNameByIndex(StringIndex_t index);
	wchar_t *GetValueByIndex(StringIndex_t index);
	const char *GetFileNameByIndex(StringIndex_t index);

	void SetValueByIndex(StringIndex_t index, wchar_t *newValue);

	int GetLocalizationFileCount(void);
	const char *GetLocalizationFileName(int index);
	bool LocalizationFileIsLoaded(const char *name);

	void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, va_list argList);

private:
	bool AddFile(IFileSystem *fileSystem, const char *fileName, const char *pPathID);
	void ReloadLocalizationFiles(void);
	void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const char *tokenName, KeyValues *dialogVariables);
	void ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, StringIndex_t unlocalizedTextSymbol, KeyValues *dialogVariables);

	bool AddAllLanguageFiles(IFileSystem *fileSystem, const char *baseFileName);

	void BuildFastValueLookup(void);
	void DiscardFastValueLookup(void);
	int FindExistingValueIndex(const wchar_t *value);

private:
	char m_szLanguage[64];
	bool m_bUseOnlyLongestLanguageString;

	struct localizedstring_t
	{
		StringIndex_t nameIndex;

		union
		{
			StringIndex_t valueIndex;
			char const *pszValueString;
		};

		CUtlSymbol filename;
	};

	CUtlRBTree<localizedstring_t, StringIndex_t> m_Lookup;

	CUtlVector<char> m_Names;
	CUtlVector<wchar_t> m_Values;
	CUtlSymbol m_CurrentFile;

	struct LocalizationFileInfo_t
	{
		CUtlSymbol symName;
		CUtlSymbol symPathID;

		static bool LessFunc(const LocalizationFileInfo_t &lhs, const LocalizationFileInfo_t &rhs)
		{
			int iresult = Q_stricmp(lhs.symPathID.String(), rhs.symPathID.String());

			if (iresult != 0)
				return iresult == -1;

			return Q_stricmp(lhs.symName.String(), rhs.symName.String()) < 0;
		}
	};

	CUtlVector<LocalizationFileInfo_t> m_LocalizationFiles;

	struct fastvalue_t
	{
		int valueindex;
		const wchar_t *search;

		static CLocalizedStringTable *s_pTable;
	};

	CUtlRBTree<fastvalue_t, int> m_FastValueLookup;

private:
	static CLocalizedStringTable *s_pTable;

	static bool SymLess(localizedstring_t const &i1, localizedstring_t const &i2);
	static bool FastValueLessFunc(const fastvalue_t &lhs, const fastvalue_t &rhs);
};

CLocalizedStringTable g_StringTable;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CLocalizedStringTable, ILocalize, VGUI_LOCALIZE_INTERFACE_VERSION, g_StringTable);

CLocalizedStringTable::CLocalizedStringTable(void) : m_Lookup(0, 0, SymLess), m_Names(1024), m_Values(2048), m_FastValueLookup(0, 0, FastValueLessFunc)
{
	m_bUseOnlyLongestLanguageString = false;
}

CLocalizedStringTable::~CLocalizedStringTable(void)
{
	m_Lookup.Purge();
	m_Names.Purge();
	m_Values.Purge();
	m_LocalizationFiles.Purge();
}

bool CLocalizedStringTable::AddFile(IFileSystem *fileSystem, const char *szFileName)
{
	return AddFile(fileSystem, szFileName, NULL);
}

bool CLocalizedStringTable::AddFile(IFileSystem *fileSystem, const char *szFileName, const char *pPathID)
{
	static const char *const LANGUAGE_STRING = "%language%";
	static const char *const ENGLISH_STRING = "english";
	static const int MAX_LANGUAGE_NAME_LENGTH = 64;
	char language[MAX_LANGUAGE_NAME_LENGTH];
	char fileName[MAX_PATH];
	int offs = 0;
	bool success = false;

	memset(language, 0, sizeof(language));
	Q_strncpy(fileName, szFileName, sizeof(fileName));

	const char *langptr = strstr(szFileName, LANGUAGE_STRING);

	if (langptr)
	{
		offs = langptr - szFileName;
		strncpy(fileName, szFileName, offs);
		fileName[offs] = 0;

		if (vgui::g_pSystem->CommandLineParamExists("-all_languages"))
		{
			m_bUseOnlyLongestLanguageString = true;
			return AddAllLanguageFiles(fileSystem, fileName);
		}

		Q_strncat(fileName, ENGLISH_STRING, sizeof(fileName), COPY_ALL_CHARACTERS);

		offs += strlen(LANGUAGE_STRING);
		Q_strncat(fileName, szFileName + offs, sizeof(fileName), COPY_ALL_CHARACTERS);
		success = AddFile(fileSystem, fileName, pPathID);

		bool bValid;

		if (IsPC())
		{
			Q_strncpy(language, gConfigs.szLanguage, sizeof(language) - 1);
			language[sizeof(language) - 1] = 0;
			bValid = true;
		}
		else
		{
			bValid = false;
		}

		if (bValid)
		{
			if (strlen(language) != 0 && stricmp(language, ENGLISH_STRING) != 0)
			{
				offs = langptr - szFileName;
				strncpy(fileName, szFileName, offs);
				fileName[offs] = 0;

				Q_strncat(fileName, language, sizeof(fileName), COPY_ALL_CHARACTERS);

				offs += strlen(LANGUAGE_STRING);
				Q_strncat(fileName, szFileName + offs, sizeof(fileName), COPY_ALL_CHARACTERS);

				success &= AddFile(fileSystem, fileName, pPathID);
			}
		}

		return success;
	}

	LocalizationFileInfo_t search;
	search.symName = fileName;
	search.symPathID = pPathID ? pPathID : "";

	int lfc = m_LocalizationFiles.Count();

	for (int lf = 0; lf < lfc; ++lf)
	{
		LocalizationFileInfo_t& entry = m_LocalizationFiles[lf];

		if (!Q_stricmp(entry.symName.String(), fileName))
		{
			m_LocalizationFiles.Remove(lf);
			break;
		}
	}

	m_LocalizationFiles.AddToTail(search);

	bool bLoadedAtLeastOne = false;
	FileHandle_t file = fileSystem->Open(fileName, "rb");

	if (file != FILESYSTEM_INVALID_HANDLE)
	{
		bLoadedAtLeastOne = true;

		m_CurrentFile = fileName;

		int fileSize = fileSystem->Size(file);
		wchar_t *memBlock = (wchar_t *)malloc(fileSize + sizeof(wchar_t));
		bool bReadOK = (fileSystem->Read(memBlock, fileSize, file) != 0);
		fileSystem->Close(file);

		memBlock[fileSize / sizeof(wchar_t)] = 0x0000;

		wchar_t *data = memBlock;
		wchar_t signature = LittleShort(data[0]);

		if (!bReadOK || signature != 0xFEFF)
		{
			Msg("Ignoring non-unicode close caption file %s\n", fileName);
			free(memBlock);
			return false;
		}

		CByteswap byteSwap;
		byteSwap.SetTargetBigEndian(false);
		byteSwap.SwapBufferToTargetEndian(data, data, fileSize / sizeof(wchar_t));

		data++;

		enum states_e
		{
			STATE_BASE,
			STATE_TOKENS,
		};

		bool bQuoted;
		bool bEnglishFile = false;

		if (strstr(fileName, "_english.txt"))
			bEnglishFile = true;

		bool spew = false;

		if (CommandLine()->CheckParm("-ccsyntax"))
			spew = true;

		BuildFastValueLookup();

		states_e state = STATE_BASE;

		while (1)
		{
			wchar_t keytoken[128];
			data = ReadUnicodeToken(data, keytoken, 128, bQuoted);

			if (!keytoken[0])
				break;

			char key[128];
			ConvertUnicodeToANSI(keytoken, key, sizeof(key));

			if (!strnicmp(key, "//", 2))
			{
				data = ReadToEndOfLine(data);
				continue;
			}

			if (spew)
				Msg("%s\n", key);

			wchar_t valuetoken[MAX_LOCALIZED_CHARS];
			data = ReadUnicodeToken(data, valuetoken, MAX_LOCALIZED_CHARS, bQuoted);

			if (!valuetoken[0] && !bQuoted)
				break;

			if (state == STATE_BASE)
			{
				if (!stricmp(key, "Language"))
				{
					char value[MAX_LOCALIZED_CHARS];
					ConvertUnicodeToANSI(valuetoken, value, sizeof(value));
					strncpy(m_szLanguage, value, sizeof(m_szLanguage) - 1);
				}
				else if (!stricmp(key, "Tokens"))
				{
					state = STATE_TOKENS;
				}
				else if (!stricmp(key, "}"))
				{
					break;
				}
			}
			else if (state == STATE_TOKENS)
			{
				if (!stricmp(key, "}"))
				{
					state = STATE_BASE;
				}
				else
				{
					if ((bEnglishFile) || (!bEnglishFile && strnicmp(key, "[english]", 9)))
					{
						AddString(key, valuetoken, NULL);
					}
				}
			}
		}

		free(memBlock);
	}

	if (!bLoadedAtLeastOne)
		Warning("ILocalize::AddFile() failed to load file \"%s\".\n", szFileName);

	DiscardFastValueLookup();
	m_CurrentFile = UTL_INVAL_SYMBOL;
	return bLoadedAtLeastOne;
}

bool CLocalizedStringTable::AddAllLanguageFiles(IFileSystem *fileSystem, const char *baseFileName)
{
	bool success = true;

	char szFilePath[MAX_PATH];
	Q_strncpy(szFilePath, baseFileName, sizeof(szFilePath));

	char *lastSlash = strrchr(szFilePath, '\\');

	if (!lastSlash)
		lastSlash = strrchr(szFilePath, '/');

	if (lastSlash)
		lastSlash[1] = 0;
	else
		szFilePath[0] = 0;

	char szSearchPath[MAX_PATH];
	Q_snprintf(szSearchPath, sizeof(szSearchPath), "%s*.txt", baseFileName);

	FileFindHandle_t hFind = NULL;
	const char *file = fileSystem->FindFirst(szSearchPath, &hFind);

	while (file)
	{
		char szFile[MAX_PATH];
		Q_snprintf(szFile, sizeof(szFile), "%s%s", szFilePath, file);

		success &= AddFile(fileSystem, szFile, NULL);
		file = fileSystem->FindNext(hFind);
	}

	fileSystem->FindClose(hFind);
	return success;
}

bool CLocalizedStringTable::SaveToFile(const char *szFileName)
{
	return SaveToFile(g_pFullFileSystem, szFileName);
}

bool CLocalizedStringTable::SaveToFile(IFileSystem *fileSystem, const char *szFileName)
{
	FileHandle_t file = fileSystem->Open(szFileName, "wb");

	if (!file)
		return false;

	CUtlSymbol fileName = szFileName;

	unsigned short marker = 0xFEFF;
	marker = LittleShort(marker);
	fileSystem->Write(&marker, sizeof(marker), file);

	const char *startStr = "\"lang\"\r\n{\r\n\"Language\" \"English\"\r\n\"Tokens\"\r\n{\r\n";
	const char *endStr = "}\r\n}\r\n";

	static wchar_t unicodeString[1024];
	int strLength = ConvertANSIToUnicode(startStr, unicodeString, sizeof(unicodeString));

	if (!strLength)
		return false;

	fileSystem->Write(unicodeString, wcslen(unicodeString) * sizeof(wchar_t), file);

	wchar_t unicodeQuote = L'\"'; 
	wchar_t unicodeCR = L'\r'; 
	wchar_t unicodeNewline = L'\n'; 
	wchar_t unicodeTab = L'\t';

	for (StringIndex_t idx = GetFirstStringIndex(); idx != INVALID_STRING_INDEX; idx = GetNextStringIndex(idx))
	{
		if (fileName != m_Lookup[idx].filename)
			continue;

		const char *name = GetNameByIndex(idx);
		wchar_t *value = GetValueByIndex(idx);

		ConvertANSIToUnicode(name, unicodeString, sizeof(unicodeString));

		fileSystem->Write(&unicodeTab, sizeof(wchar_t), file);
		fileSystem->Write(&unicodeQuote, sizeof(wchar_t), file);
		fileSystem->Write(unicodeString, wcslen(unicodeString) * sizeof(wchar_t), file);
		fileSystem->Write(&unicodeQuote, sizeof(wchar_t), file);

		fileSystem->Write(&unicodeTab, sizeof(wchar_t), file);
		fileSystem->Write(&unicodeTab, sizeof(wchar_t), file);

		fileSystem->Write(&unicodeQuote, sizeof(wchar_t), file);
		fileSystem->Write(value, wcslen(value) * sizeof(wchar_t), file);
		fileSystem->Write(&unicodeQuote, sizeof(wchar_t), file);

		fileSystem->Write(&unicodeCR, sizeof(wchar_t), file);
		fileSystem->Write(&unicodeNewline, sizeof(wchar_t), file);
	}

	strLength = ConvertANSIToUnicode(endStr, unicodeString, sizeof(unicodeString));
	fileSystem->Write(unicodeString, strLength * sizeof(wchar_t), file);

	fileSystem->Close(file);
	return true;
}

void CLocalizedStringTable::ReloadLocalizationFiles(void)
{
	for (int i = 0; i < m_LocalizationFiles.Count(); i++)
	{
		LocalizationFileInfo_t &entry = m_LocalizationFiles[i];

		AddFile
		(
			g_pFullFileSystem,
			entry.symName.String(), 
			entry.symPathID.String()[0] ? entry.symPathID.String() : NULL
		);
	}
}

bool CLocalizedStringTable::SymLess(localizedstring_t const &i1, localizedstring_t const &i2)
{
	const char *str1 = (i1.nameIndex == INVALID_STRING_INDEX) ? i1.pszValueString : &g_StringTable.m_Names[i1.nameIndex];
	const char *str2 = (i2.nameIndex == INVALID_STRING_INDEX) ? i2.pszValueString : &g_StringTable.m_Names[i2.nameIndex];
	return stricmp(str1, str2) < 0;
}

wchar_t *CLocalizedStringTable::Find(const char *pName)
{
	StringIndex_t idx = FindIndex(pName);

	if (idx == INVALID_STRING_INDEX)
		return NULL;

	return &m_Values[m_Lookup[idx].valueIndex];
}

StringIndex_t CLocalizedStringTable::FindIndex(const char *pName)
{
	if (!pName)
		return NULL;

	if (pName[0] == '#')
		pName++;

	localizedstring_t invalidItem;
	invalidItem.nameIndex = INVALID_STRING_INDEX;
	invalidItem.pszValueString = pName;
	return m_Lookup.Find(invalidItem);
}

void CLocalizedStringTable::AddString(const char *pString, wchar_t *pValue, const char *fileName)
{
	if (!pString)
		return;

	MEM_ALLOC_CREDIT();

	int valueIndex = FindExistingValueIndex(pValue);

	if (valueIndex == INVALID_STRING_INDEX)
	{
		int len = wcslen(pValue) + 1;
		valueIndex = m_Values.AddMultipleToTail(len);
		memcpy(&m_Values[valueIndex], pValue, len * sizeof(wchar_t));
	}

	StringIndex_t stridx = FindIndex(pString);
	localizedstring_t item;
	item.nameIndex = stridx;

	if (stridx == INVALID_STRING_INDEX)
	{
		int len = strlen(pString) + 1;
		stridx = m_Names.AddMultipleToTail(len);
		memcpy(&m_Names[stridx], pString, len * sizeof(char));

		item.nameIndex = stridx;
		item.valueIndex = valueIndex;
		item.filename = fileName ? fileName : m_CurrentFile;

		m_Lookup.Insert(item);
	}
	else
	{
		if (m_bUseOnlyLongestLanguageString)
		{
			wchar_t *newValue = pValue;
			wchar_t *oldValue = GetValueByIndex(stridx);

			int newWide, oldWide, tall;
			vgui::g_pSurface->GetTextSize(1, newValue, newWide, tall);
			vgui::g_pSurface->GetTextSize(1, oldValue, oldWide, tall);

			if (newWide < oldWide)
				return;
		}

		item.nameIndex = GetNameByIndex(stridx) - &m_Names[0];
		item.valueIndex = valueIndex;
		item.filename = fileName ? fileName : m_CurrentFile;
		m_Lookup[stridx] = item;
	}
}

void CLocalizedStringTable::RemoveAll(void)
{
	m_Lookup.RemoveAll();
	m_Names.RemoveAll();
	m_Values.RemoveAll();
	m_LocalizationFiles.RemoveAll();
}

StringIndex_t CLocalizedStringTable::GetFirstStringIndex()
{
	return m_Lookup.FirstInorder();
}

StringIndex_t CLocalizedStringTable::GetNextStringIndex(StringIndex_t index)
{
	StringIndex_t idx = m_Lookup.NextInorder(index);

	if (idx == m_Lookup.InvalidIndex())
		return INVALID_STRING_INDEX;

	return idx;
}

const char *CLocalizedStringTable::GetNameByIndex(StringIndex_t index)
{
	localizedstring_t &lstr = m_Lookup[index];
	return &m_Names[lstr.nameIndex];
}

wchar_t *CLocalizedStringTable::GetValueByIndex(StringIndex_t index)
{
	if (index == INVALID_STRING_INDEX)
		return NULL;

	localizedstring_t &lstr = m_Lookup[index];
	return &m_Values[lstr.valueIndex];
}

CLocalizedStringTable *CLocalizedStringTable::s_pTable = NULL;

bool CLocalizedStringTable::FastValueLessFunc(const fastvalue_t &lhs, const fastvalue_t &rhs)
{
	Assert(s_pTable);

	const wchar_t *w1 = lhs.search ? lhs.search : &s_pTable->m_Values[lhs.valueindex];
	const wchar_t *w2 = rhs.search ? rhs.search : &s_pTable->m_Values[rhs.valueindex];

	return (wcscmp(w1, w2) < 0) ? true : false;
}

void CLocalizedStringTable::BuildFastValueLookup(void)
{
	m_FastValueLookup.RemoveAll();
	s_pTable = this;

	int c = m_Lookup.Count();

	for (int i = 0; i < c; ++i)
	{
		fastvalue_t val;
		val.valueindex = m_Lookup[i].valueIndex;
		val.search = NULL;

		m_FastValueLookup.Insert(val);
	}
}

void CLocalizedStringTable::DiscardFastValueLookup(void)
{
	m_FastValueLookup.RemoveAll();
	s_pTable = NULL;
}

int CLocalizedStringTable::FindExistingValueIndex(const wchar_t *value)
{
	if (!s_pTable)
		return INVALID_STRING_INDEX;

	fastvalue_t val;
	val.valueindex = -1;
	val.search = value;

	int idx = m_FastValueLookup.Find(val);

	if (idx != m_FastValueLookup.InvalidIndex())
		return m_FastValueLookup[idx].valueindex;

	return INVALID_STRING_INDEX;
}

const char *CLocalizedStringTable::GetFileNameByIndex(StringIndex_t index)
{
	localizedstring_t &lstr = m_Lookup[index];
	return lstr.filename.String();
}

void CLocalizedStringTable::SetValueByIndex(StringIndex_t index, wchar_t *newValue)
{
	localizedstring_t &lstr = m_Lookup[index];
	wchar_t *wstr = &m_Values[lstr.valueIndex];

	int newLen = wcslen(newValue);
	int oldLen = wcslen(wstr);

	if (newLen > oldLen)
	{
		lstr.valueIndex = m_Values.AddMultipleToTail(newLen + 1);
		memcpy(&m_Values[lstr.valueIndex], newValue, (newLen + 1) * sizeof(wchar_t));
	}
	else
	{
		wcscpy(wstr, newValue);
	}
}

int CLocalizedStringTable::GetLocalizationFileCount(void)
{
	return m_LocalizationFiles.Count();
}

const char *CLocalizedStringTable::GetLocalizationFileName(int index)
{
	return m_LocalizationFiles[index].symName.String();
}

bool CLocalizedStringTable::LocalizationFileIsLoaded(const char *name)
{
	int c = m_LocalizationFiles.Count();

	for (int i = 0; i < c; ++i)
	{
		if (!Q_stricmp(m_LocalizationFiles[i].symName.String(), name))
			return true;
	}

	return false;
}

int CLocalizedStringTable::ConvertANSIToUnicode(const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes)
{
	int chars = ::MultiByteToWideChar(CP_UTF8, 0, ansi, -1, unicode, unicodeBufferSizeInBytes / sizeof(wchar_t));
	unicode[(unicodeBufferSizeInBytes / sizeof(wchar_t)) - 1] = 0;
	return chars;
}

int CLocalizedStringTable::ConvertUnicodeToANSI(const wchar_t *unicode, char *ansi, int ansiBufferSize)
{
	int result = ::WideCharToMultiByte(CP_UTF8, 0, unicode, -1, ansi, ansiBufferSize, NULL, NULL);
	ansi[ansiBufferSize - 1] = 0;
	return result;
}

#define va_argByIndex(ap, t, i) (*(t *)(ap + i * _INTSIZEOF(t)))

void CLocalizedStringTable::ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, ...)
{
	if (!formatString)
	{
		unicodeOutput[0] = 0;
		return;
	}

	va_list argList;
	va_start(argList, numFormatParameters);
	ConstructString(unicodeOutput, unicodeBufferSizeInBytes, formatString, numFormatParameters, argList);
	va_end(argList);
}

void CLocalizedStringTable::ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, va_list argList)
{
	int unicodeBufferSize = unicodeBufferSizeInBytes / sizeof(wchar_t);
	wchar_t *searchPos = formatString;
	wchar_t *outputPos = unicodeOutput;
	int formatLength = wcslen(formatString);
#ifdef _DEBUG
	int curArgIdx = 0;
#endif

	while (searchPos[0] != '\0' && unicodeBufferSize > 0)
	{
		if (formatLength >= 3 && searchPos[0] == '%' && searchPos[1] == 's')
		{
			int argindex = (searchPos[2]) - '0' - 1;

			if (argindex < 0 || argindex > 9)
			{
				Warning("Bad format string in CLocalizeStringTable::ConstructString\n");
				*outputPos = '\0';
				return;
			}

			if (argindex < numFormatParameters)
			{
				wchar_t *param = NULL;

				if (IsPC())
				{
					param = va_argByIndex(argList, wchar_t *, argindex);
				}
				else
				{
					Assert(argindex == curArgIdx++);
					param = va_arg(argList, wchar_t *);
				}

				if (!param)
				{
					Assert(!("CLocalizedStringTable::ConstructString - Found a %s# escape sequence who's index was more than the number of args."));
					*outputPos = '\0';
				}

				int paramSize = wcslen(param);

				if (paramSize > unicodeBufferSize)
					paramSize = unicodeBufferSize;

				wcsncpy(outputPos, param, paramSize);

				unicodeBufferSize -= paramSize;
				outputPos += paramSize;

				searchPos += 3;
				formatLength -= 3;
			}
			else
			{
				*outputPos = *searchPos;

				outputPos++;
				unicodeBufferSize--;

				searchPos++;
				formatLength--;
			}
		}
		else
		{
			*outputPos = *searchPos;

			outputPos++;
			unicodeBufferSize--;

			searchPos++;
			formatLength--;
		}
	}

	*outputPos = '\0';
}

void CLocalizedStringTable::ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const char *tokenName, KeyValues *localizationVariables)
{
	StringIndex_t index = FindIndex(tokenName);

	if (index != INVALID_STRING_INDEX)
		ConstructString(unicodeOutput, unicodeBufferSizeInBytes, index, localizationVariables);
	else
		ConvertANSIToUnicode(tokenName, unicodeOutput, unicodeBufferSizeInBytes);
}

void CLocalizedStringTable::ConstructString(wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, StringIndex_t unlocalizedTextSymbol, KeyValues *localizationVariables)
{
	if (unicodeBufferSizeInBytes < 1)
		return;

	unicodeOutput[0] = 0;
	const wchar_t *searchPos = GetValueByIndex(unlocalizedTextSymbol);

	if (!searchPos)
	{
		wcsncpy(unicodeOutput, L"[unknown string]", unicodeBufferSizeInBytes / sizeof(wchar_t));
		return;
	}

	wchar_t *outputPos = unicodeOutput;
	int unicodeBufferSize = unicodeBufferSizeInBytes / sizeof(wchar_t);

	while (*searchPos != '\0' && unicodeBufferSize > 0)
	{
		bool shouldAdvance = true;

		if (*searchPos == '%')
		{
			if (searchPos[1] == 's' && searchPos[2] >= '0' && searchPos[2] <= '9')
			{
			}
			else if (searchPos[1] == '%')
			{
				searchPos++;
			}
			else if (localizationVariables)
			{
				const wchar_t *varStart = searchPos + 1;
				const wchar_t *varEnd = wcschr(varStart, '%');

				if (varEnd && *varEnd == '%')
				{
					shouldAdvance = false;

					char variableName[32];
					char *vset = variableName;

					for (const wchar_t *pws = varStart; pws < varEnd && (vset < variableName + sizeof(variableName) - 1); ++pws, ++vset)
						*vset = (char)*pws;

					*vset = 0;

					const wchar_t *value = localizationVariables->GetWString(variableName, L"[unknown]");
					int paramSize = wcslen(value);

					if (paramSize > unicodeBufferSize)
						paramSize = unicodeBufferSize;

					wcsncpy(outputPos, value, paramSize);

					unicodeBufferSize -= paramSize;
					outputPos += paramSize;
					searchPos = varEnd + 1;
				}
			}
		}

		if (shouldAdvance)
		{
			*outputPos = *searchPos;

			outputPos++;
			unicodeBufferSize--;

			searchPos++;
		}
	}

	*outputPos = '\0';
}