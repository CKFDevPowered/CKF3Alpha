#define DIB_HEADER_MARKER ((WORD)('M' << 8) | 'B')

int LoadDDS(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
int LoadBMP(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
int LoadPNG(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
int LoadJPEG(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
void GL_UploadDXT(byte *data, int width, int height, qboolean mipmap, qboolean ansio);