#include "bmp.h" //	Simple .bmp library
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>

using namespace std;
#define PI 3.14159265358979
int QuantizationMatrix[8][8] = {
	{3, 5, 7, 9, 11, 13, 15, 17},
	{5, 7, 9, 11, 13, 15, 17, 19},
	{7, 9, 11, 13, 15, 17, 19, 21},
	{9, 11, 13, 15, 17, 19, 21, 23},
	{11, 13, 15, 17, 19, 21, 23, 25},
	{13, 15, 17, 19, 21, 23, 25, 27},
	{15, 17, 19, 21, 23, 25, 27, 29},
	{17, 19, 21, 23, 25, 27, 29, 31}};

double c(int k)
{
	return k != 0 ? 1 : 1 / sqrt(2);
}

double **createDoubleArray(int m, int n)
{
	double *values = (double *)calloc(m * n, sizeof(double));
	double **rows = (double **)malloc(m * sizeof(double *));
	for (int i = 0; i < m; ++i)
	{
		rows[i] = values + i * n;
	}
	return rows;
}

double **resizedImage(double **inputArry, int inputWidth, int inputHeight, int width, int height)
{
	double **resizedArr = createDoubleArray(height + 1, width + 1);
	double x_ratio = (inputWidth - 1) * 1.0f / (width - 1);
	double y_ratio = (inputHeight - 1) * 1.0f / (height - 1);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{

			int x_l = floor(x_ratio * j);
			int y_l = floor(y_ratio * i);
			int x_h = ceil(x_ratio * j);
			int y_h = ceil(y_ratio * i);

			double x_weight = x_ratio * j - x_l;
			double y_weight = y_ratio * i - y_l;

			double a = inputArry[y_l][x_l];
			double b = inputArry[y_l][x_h];
			double c = inputArry[y_h][x_l];
			double d = inputArry[y_h][x_h];

			double pixel = a * (1 - x_weight) * (1 - y_weight) + b * x_weight * (1 - y_weight) +
						   c * y_weight * (1 - x_weight) +
						   d * x_weight * y_weight;

			resizedArr[i][j] = pixel;
		}
	}
	return resizedArr;
}

int main(int argc, char **argv)
{
	if (argc <= 2 || argc >= 5)
	{
		cout << "Arguments prompt: dct.exe <img_path> <apply_idct> <scale>" << endl;
		return 0;
	}
	string imgPath = argv[1];
	bool need_idct = stoi(argv[2]);
	float scale = 1;
	if (argc == 4)
	{
		scale = stof(argv[3]);
		if (scale <= 0)
			scale = 1;
	}

	//! read input image
	Bitmap s_img(imgPath.c_str());
	int rows = s_img.getHeight(), cols = s_img.getWidth();
	cout << "Apply DCT on image (" << rows << ", " << cols << ")." << endl;

	//! preprocess by shifting pixel values by 128

	//! 2D DCT for every 8x8 block (assume that the input image resolution is fixed to 256)
	// The quantized coefficients stored into 'coeffArray'
	double coeffArray[256][256] = {0};
	int blockRow = rows / 8, blockCol = cols / 8;
	for (int i = 0; i < blockRow; i++)
	{
		for (int j = 0; j < blockCol; j++)
		{
			int xpos = j * 8, ypos = i * 8;
			double F_r[8][8];
			for (int u = 0; u < 8; u++)
			{
				for (int v = 0; v < 8; v++)
				{
					double rowSum = 0;
					for (int x = 0; x < 8; x++)
					{
						unsigned char val;
						s_img.getPixel(xpos + x, ypos + v, val);
						rowSum += cos((2 * x + 1) * u * PI / 16) * (val - 128);
					}
					F_r[u][v] = rowSum * c(u) / 2;
				}
			}
			for (int u = 0; u < 8; u++)
			{
				for (int v = 0; v < 8; v++)
				{
					double colSum = 0;
					for (int y = 0; y < 8; y++)
					{
						colSum += cos((2 * y + 1) * v * PI / 16) * F_r[u][y];
					}
					coeffArray[xpos + u][ypos + v] = c(v) * colSum / 2;
				}
			}
			// quantize the frequency coefficient of this block
			for (int u = 0; u < 8; u++)
			{
				for (int v = 0; v < 8; v++)
				{
					coeffArray[u + xpos][v + ypos] = round(coeffArray[u + xpos][v + ypos] / QuantizationMatrix[u][v]);
				}
			}
		}
	}

	//! output the computed coefficient array
	FILE *fp = fopen("coeffs.txt", "w");
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			fprintf(fp, "%3.3lf ", coeffArray[c][r]);
		}
		fprintf(fp, "\n");
	}
	cout << "Quantized coefficients saved!" << endl;

	if (need_idct)
	{
		Bitmap reconstructedImg(cols, rows);
		double **ImageArr = createDoubleArray(rows, cols);
		for (int i = 0; i < blockRow; i++)
		{
			for (int j = 0; j < blockCol; j++)
			{
				int xpos = j * 8, ypos = i * 8;

				// apply de-quantization on block_ij
				double F_1[8][8];
				for (int u = 0; u < 8; u++)
				{
					for (int v = 0; v < 8; v++)
					{
						F_1[u][v] = coeffArray[xpos + u][ypos + v] * QuantizationMatrix[u][v];
					}
				}

				// apply IDCT on this block
				double F_2[8][8];
				for (int u = 0; u < 8; u++)
				{
					for (int v = 0; v < 8; v++)
					{
						double colSum = 0;
						for (int y = 0; y < 8; y++)
						{
							colSum += cos((2 * v + 1) * y * PI / 16) * F_1[u][y] * c(y);
						}
						F_2[u][v] = colSum / 2;
					}
				}

				double F_3[8][8];
				for (int u = 0; u < 8; u++)
				{
					for (int v = 0; v < 8; v++)
					{
						double rowSum = 0;
						for (int x = 0; x < 8; x++)
						{
							rowSum += cos((2 * u + 1) * x * PI / 16) * F_2[x][v] * c(x);
						}
						F_3[u][v] = rowSum / 2;
					}
				}

				// shiftting back the pixel value range to 0~255
				for (int u = 0; u < 8; u++)
				{
					for (int v = 0; v < 8; v++)
					{
						ImageArr[xpos + u][ypos + v] = round(F_3[u][v] + 128);
					}
				}
			}
		}

		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				if (ImageArr[i][j] > 255)
					ImageArr[i][j] = 255;
				if (ImageArr[i][j] < 0)
					ImageArr[i][j] = 0;

				reconstructedImg.setPixel(i, j, ImageArr[i][j]);
			}
		}

		if (scale != 1)
		{
			int size = (int)(scale * rows);
			double **newArr = resizedImage(ImageArr, cols - 1, rows - 1, size, size);
			Bitmap newImg = Bitmap(size, size);
			for (int i = 0; i < size; i++)
				for (int j = 0; j < size; j++)
				{
					if (newArr[i][j] > 255)
						newArr[i][j] = 255;
					if (newArr[i][j] < 0)
						newArr[i][j] = 0;
					newImg.setPixel(i, j, (unsigned char)newArr[i][j]);
				}
			newImg.save("reconstructedImg.bmp");
			return 0;
		}

		string savePath = "reconstructedImg.bmp";
		reconstructedImg.save(savePath.c_str());
		cout << "reconstructed image saved!" << endl;
	}

	return 0;
}