#include "bmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <algorithm>
#include<iostream>

using namespace std;

#define SafeFree(p)  { if(p) { free(p); p = NULL; } }

struct RGBQUAD
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char QUAD;
};

Bitmap::Bitmap()
{
	memset(this, 0, sizeof(Bitmap));
}


Bitmap::Bitmap(const char *path)
{
	memset(this, 0, sizeof(Bitmap));
	create(path);
}



Bitmap::Bitmap(int width, int height)
{
	memset(this, 0, sizeof(Bitmap));
	create(width, height);
}



Bitmap::~Bitmap()
{
//	SafeFree(_data);
}


bool Bitmap::create(int width, int height)
{

	SafeFree(_data);

	_width = width;
	_height = height;

	_data = (unsigned char *)malloc(_width * _height * sizeof(unsigned char));
	if (_data == NULL)
		return false;
	memset(_data, 0, _width * _height * sizeof(unsigned char)); 

	return true;

}

bool Bitmap::create(const char *path)
{
	BmpHeaderInfo bhi;

	{
		FILE *fp = fopen(path, "rb");
		if (fp == NULL)
			return false;

		fread(&bhi, sizeof(BmpHeaderInfo), 1, fp);
		fclose(fp);
	}

	if (bhi.bfType != 'MB')
		return false;

	if (bhi.biCompression != 0)
		return false;

	if (bhi.biBitCount != 8)
		return false;

	if (!create(bhi.biWidth, bhi.biHeight))
		return false;

	{
		FILE *fp = fopen(path, "rb");
		fseek(fp, bhi.bfOffBits, SEEK_SET);

		int index = 0;
		for (int j = 0; j < _height; j++)
		{
			fread(&_data[index], sizeof(unsigned char), _width, fp);
			fseek(fp, (4 - _width % 4) % 4, SEEK_CUR);
			index += _width;
		}
		fclose(fp);
	}
	return true;
}


bool Bitmap::save(const char *path)
{
	BmpHeaderInfo bhi;
	RGBQUAD	palette[256];

	for (int i=0; i<256; i++)
	{
		palette[i].R = i;
		palette[i].G = i;
		palette[i].B = i;
		palette[i].QUAD = 0;
	}

	bhi.bfType = 'MB';
	bhi.bfSize = _width * _height * sizeof(unsigned char) + sizeof(bhi) + sizeof(palette);
	bhi.bfReserved1 = 0;
	bhi.bfReserved2 = 0;
	bhi.bfOffBits = sizeof(bhi) + sizeof(palette);

	bhi.biSize = 40;
	bhi.biWidth = _width;
	bhi.biHeight = _height;
	bhi.biPlanes = 1;
	bhi.biBitCount = 8;
	bhi.biCompression = 0;
	bhi.biSizeImage = 0;
	bhi.biXpelsPerMeter = 0;
	bhi.biYpelsPerMeter = 0;
	bhi.biClrUsed = 0;
	bhi.biClrImportant = 0;

	unsigned char pad = 0;

	FILE * fp = fopen(path, "wb");
	if (fp == NULL) return false;

	fwrite(&bhi, sizeof(BmpHeaderInfo), 1, fp);
	fwrite(palette, sizeof(RGBQUAD), 256, fp);
	int index = 0;
	for (int j = 0; j < _height; j++) {
		
		fwrite(&_data[index], sizeof(unsigned char), _width, fp);
		fwrite(&pad, sizeof(unsigned char), (4 - _width % 4) % 4, fp);
		index += _width;
	}
	fclose(fp);

	return(true);

}
