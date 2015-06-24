inline void Warning(char *s, ...) {}

inline void Msg(char *s, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, s);
	vsprintf(buffer, s, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

inline void Msg(wchar_t *s, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static wchar_t buffer[1024];

	va_start(argptr, s);
	vswprintf(buffer, s, argptr);
	va_end(argptr);

	OutputDebugStringW(buffer);
#endif
}

inline void Error(char *s, ...) {}