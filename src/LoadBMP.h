#define DIB_HEADER_MARKER ((WORD)('M' << 8) | 'B')

int LoadBMP(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
int LoadPNG(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);