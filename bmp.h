#ifndef _BITMAP_H_
#define _BITMAP_H_


class Bitmap
{
public:
	Bitmap();
	Bitmap(const char *path);
	Bitmap(int width, int height);

	~Bitmap();

	bool create(const char *path);
	bool create(int width, int height);

	bool save(const char *path);
	bool resize(int maxlen);

	inline unsigned char * getData();

	inline int getWidth();
	inline int getHeight();

	inline void getPixel(int x, int y, unsigned char &value);
	inline void setPixel(int x, int y, unsigned char value);

#pragma pack(push, 1)
	struct BmpHeaderInfo
	{
		// BMP File Header
		unsigned short bfType;
		unsigned int bfSize;
		unsigned short bfReserved1;
		unsigned short bfReserved2;
		unsigned int bfOffBits;

		// Bitmap Information
		unsigned int biSize;
		int biWidth;
		int biHeight;
		unsigned short biPlanes;
		unsigned short biBitCount;
		unsigned int biCompression;
		unsigned int biSizeImage;
		int biXpelsPerMeter;
		int biYpelsPerMeter;
		unsigned int biClrUsed;
		unsigned int biClrImportant;
	};
#pragma pack(pop)

	int _width;
	int _height;
	unsigned char *_data;
};

unsigned char * Bitmap::getData()
{
	return _data;
}

int Bitmap::getWidth()
{
	return _width;
}

int Bitmap::getHeight()
{
	return _height;
}

void Bitmap::getPixel(int x, int y, unsigned char &value)
{
	int pos = x + y * _width;
	value = _data[pos];
}

void Bitmap::setPixel(int x, int y, unsigned char value)
{

	int pos = x + y * _width;
	_data[pos] = value;
	
}

#endif