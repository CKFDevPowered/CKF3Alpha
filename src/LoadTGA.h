int LoadTGA(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
bool LoadTGA2(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
bool GetTGASize(const char *szFilename, int *width, int *height);
bool WriteTGA(byte *pixels, int width, int height, const char *szFilename);