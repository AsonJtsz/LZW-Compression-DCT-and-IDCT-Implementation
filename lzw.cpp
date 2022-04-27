#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>

#define CODE_SIZE 12
#define TRUE 1
#define FALSE 0

typedef struct dict
{
	int character;
	int previous_character;
} dict;

/* function prototypes */
unsigned int read_code(FILE *, unsigned int);
void write_code(FILE *, unsigned int, unsigned int);
void writefileheader(FILE *, char **, int);
void readfileheader(FILE *, char **, int *);
void compress(FILE *, FILE *);
void compress2(FILE *, FILE *);
void compress_enhanced(FILE *, FILE *);
void decompress(FILE *, FILE *);
void decompress2(FILE *input, FILE *output);
void decompress_enhanced(FILE *, FILE *);
void init_table();
int search_table(int, int);
int first_char(int, int);

dict table[4096];
int table_idx;
int write_code_count = 0;

std::map<std::string, int> entry_map;
std::string reverse_map[4096];
int reverse_map_count;
bool end_reading = false;

void init_table()
{
	for (table_idx = 0; table_idx < 256; table_idx++)
	{
		table[table_idx].character = table_idx;
		table[table_idx].previous_character = -1;
	}
	table[4095].character = EOF;
	table[4095].previous_character = -1;
}

void init_map()
{
	entry_map.clear();
	for (int i = 0; i < 256; i++)
	{
		entry_map[std::string(1, (char)i)] = i;
	}
}

void init_reverse_map()
{
	std::fill(reverse_map, reverse_map + 4096, "");
	for (int i = 0; i < 256; i++)
	{
		reverse_map[i] = std::string(1, (char)i);
	}
	reverse_map_count = 256;
}

int search_table(int p, int c)
{
	int return_val = -1;

	for (int i = 0; i < table_idx; i++)
	{
		if (table[i].previous_character == p && table[i].character == c)
		{
			return_val = i;
			break;
		}
	}
	return return_val;
}

void add_table(int p, int c)
{
	// if (table_idx == 4096)
	// 	init_table();
	table[table_idx].character = c;
	table[table_idx].previous_character = p;
	table_idx += 1;
}

int first_char(int index)
{
	if (table[index].previous_character == -1)
	{
		return table[index].character;
	}
	else
	{
		return first_char(table[index].previous_character);
	}
}

void print_string(FILE *output, int index)
{
	if (table[index].previous_character != -1)
	{
		print_string(output, table[index].previous_character);
	}

	fputc(table[index].character, output);
}

void print_string(std::string s, FILE *output)
{
	for (int i = 0; i < s.length(); i++)
	{
		fputc(s[i], output);
	}
}

int main(int argc, char **argv)
{
	int printusage = 0;
	int no_of_file;
	char **input_file_names;
	char *output_file_names;
	FILE *lzw_file;

	if (argc >= 3)
	{
		if (strcmp(argv[1], "-c") == 0)
		{
			/* compression */
			lzw_file = fopen(argv[2], "wb");

			/* write the file header */
			input_file_names = argv + 3;
			no_of_file = argc - 3;
			writefileheader(lzw_file, input_file_names, no_of_file);

			init_table();
			for (int i = 0; i < no_of_file; i++)
			{
				FILE *file_read = fopen(argv[i + 3], "rb");
				compress(file_read, lzw_file);
				printf("Adding: %s\n", argv[3 + i]);
			}

			int buffer = write_code_count;
			if (buffer != 0)
			{
				write_code(lzw_file, 0, buffer);
			}
			else
			{
				write_code(lzw_file, 0, CODE_SIZE);
			}

			fclose(lzw_file);
		}
		else if (strcmp(argv[1], "-d") == 0)
		{
			/* decompress */
			lzw_file = fopen(argv[2], "rb");

			/* read the file header */
			no_of_file = 0;
			readfileheader(lzw_file, &output_file_names, &no_of_file);

			init_table();
			char *file_ptr = strtok(output_file_names, "\n");
			for (int i = 0; i < no_of_file; i++)
			{
				FILE *output_file = fopen(file_ptr, "wb");
				decompress(lzw_file, output_file);
				printf("Deflating: %s\n", file_ptr);
				file_ptr = strtok(nullptr, "\n");
			}

			fclose(lzw_file);
			free(output_file_names);
		}
		else if (strcmp(argv[1], "-C") == 0)
		{
			lzw_file = fopen(argv[2], "wb");

			input_file_names = argv + 3;
			no_of_file = argc - 3;
			writefileheader(lzw_file, input_file_names, no_of_file);

			init_map();
			for (int i = 0; i < no_of_file; i++)
			{
				FILE *file_read = fopen(argv[i + 3], "rb");
				compress2(file_read, lzw_file);
				printf("Adding: %s\n", argv[3 + i]);
			}

			int buffer = write_code_count;
			if (buffer != 0)
			{
				write_code(lzw_file, 0, buffer);
			}
			else
			{
				write_code(lzw_file, 0, CODE_SIZE);
			}

			fclose(lzw_file);
		}
		else if (strcmp(argv[1], "-D") == 0)
		{
			lzw_file = fopen(argv[2], "rb");

			no_of_file = 0;
			readfileheader(lzw_file, &output_file_names, &no_of_file);

			init_reverse_map();
			char *file_ptr = strtok(output_file_names, "\n");
			for (int i = 0; i < no_of_file; i++)
			{
				FILE *output_file = fopen(file_ptr, "wb");
				decompress2(lzw_file, output_file);
				printf("Deflating: %s\n", file_ptr);
				file_ptr = strtok(nullptr, "\n");
			}

			fclose(lzw_file);
			free(output_file_names);
		}
		else if (strcmp(argv[1], "-CC") == 0)
		{
			lzw_file = fopen(argv[2], "wb");

			input_file_names = argv + 3;
			no_of_file = argc - 3;
			writefileheader(lzw_file, input_file_names, no_of_file);

			init_map();
			for (int i = 0; i < no_of_file; i++)
			{
				FILE *file_read = fopen(argv[i + 3], "rb");
				compress_enhanced(file_read, lzw_file);
				printf("Adding: %s\n", argv[3 + i]);
			}

			int buffer = write_code_count;
			if (buffer != 0)
			{
				write_code(lzw_file, 0, buffer);
			}
			else
			{
				int code_size;
				int size = entry_map.size();
				if (size <= 511)
				{
					code_size = 9;
				}
				else if (size <= 1023)
				{
					code_size = 10;
				}
				else if (size <= 2047)
				{
					code_size = 11;
				}
				else
				{
					code_size = 12;
				}
				write_code(lzw_file, 0, code_size);
			}

			fclose(lzw_file);
		}
		else if (strcmp(argv[1], "-DD") == 0)
		{
			lzw_file = fopen(argv[2], "rb");

			no_of_file = 0;
			readfileheader(lzw_file, &output_file_names, &no_of_file);

			init_reverse_map();
			char *file_ptr = strtok(output_file_names, "\n");
			for (int i = 0; i < no_of_file; i++)
			{
				FILE *output_file = fopen(file_ptr, "wb");
				decompress_enhanced(lzw_file, output_file);
				printf("Deflating: %s\n", file_ptr);
				file_ptr = strtok(nullptr, "\n");
			}

			fclose(lzw_file);
			free(output_file_names);
		}
		else
			printusage = 1;
	}
	else
		printusage = 1;

	if (printusage)
		printf("Usage: %s -<c/d> <lzw filename> <list of files>\n", argv[0]);

	return 0;
}

/*****************************************************************
 *
 * writefileheader() -  write the lzw file header to support multiple files
 *
 ****************************************************************/
void writefileheader(FILE *lzw_file, char **input_file_names, int no_of_files)
{
	int i;
	/* write the file header */
	for (i = 0; i < no_of_files; i++)
	{
		fprintf(lzw_file, "%s\n", input_file_names[i]);
	}
	fputc('\n', lzw_file);
}

/*****************************************************************
 *
 * readfileheader() - read the fileheader from the lzw file
 *
 ****************************************************************/
void readfileheader(FILE *lzw_file, char **output_filenames, int *no_of_files)
{
	int noofchar;
	char c, lastc;

	noofchar = 0;
	lastc = 0;
	*no_of_files = 0;
	/* find where is the end of double newline */
	while ((c = fgetc(lzw_file)) != EOF)
	{
		noofchar++;
		if (c == '\n')
		{
			if (lastc == c)
				/* found double newline */
				break;
			(*no_of_files)++;
		}
		lastc = c;
	}

	if (c == EOF)
	{
		/* problem .... file may have corrupted*/
		*no_of_files = 0;
		return;
	}
	/* allocate memeory for the filenames */
	*output_filenames = (char *)malloc(sizeof(char) * noofchar);
	/* roll back to start */
	fseek(lzw_file, 0, SEEK_SET);

	fread((*output_filenames), 1, (size_t)noofchar, lzw_file);

	return;
}

/*****************************************************************
 *
 * read_code() - reads a specific-size code from the code file
 *
 ****************************************************************/
unsigned int read_code(FILE *input, unsigned int code_size)
{
	unsigned int return_value;
	static int input_bit_count = 0;
	static unsigned long input_bit_buffer = 0L;

	/* The code file is treated as an input bit-stream. Each     */
	/*   character read is stored in input_bit_buffer, which     */
	/*   is 32-bit wide.                                         */

	/* input_bit_count stores the no. of bits left in the buffer */

	while (input_bit_count <= 24)
	{
		int c = getc(input);
		if (c == EOF)
		{
			end_reading = true;
		}

		input_bit_buffer |= (unsigned long)c << (24 - input_bit_count);
		input_bit_count += 8;
	}

	return_value = input_bit_buffer >> (32 - code_size);
	input_bit_buffer <<= code_size;
	input_bit_count -= code_size;

	return (return_value);
}

/*****************************************************************
 *
 * write_code() - write a code (of specific length) to the file
 *
 ****************************************************************/
void write_code(FILE *output, unsigned int code, unsigned int code_size)
{
	static int output_bit_count = 0;
	static unsigned long output_bit_buffer = 0L;

	/* Each output code is first stored in output_bit_buffer,    */
	/*   which is 32-bit wide. Content in output_bit_buffer is   */
	/*   written to the output file in bytes.                    */

	/* output_bit_count stores the no. of bits left              */

	output_bit_buffer |= (unsigned long)code << (32 - code_size - output_bit_count);
	output_bit_count += code_size;

	while (output_bit_count >= 8)
	{
		putc(output_bit_buffer >> 24, output);
		output_bit_buffer <<= 8;
		output_bit_count -= 8;
	}
	write_code_count = (write_code_count + code_size) % 8;
	/* only < 8 bits left in the buffer                          */
}

/*****************************************************************
 *
 * compress() - compress the source file and output the coded text
 *
 ****************************************************************/
void compress(FILE *input, FILE *output)
{
	int Cu, N;

	Cu = fgetc(input);
	if (Cu == EOF)
		return;
	N = fgetc(input);
	if (N == EOF)
		return;

	while (N != EOF)
	{
		int code = search_table(Cu, N);
		if (code != -1)
		{
			Cu = code;
			N = fgetc(input);
		}
		else
		{
			write_code(output, Cu, CODE_SIZE);
			add_table(Cu, N);

			Cu = N;
			N = fgetc(input);
		}

		if (table_idx >= 4096)
		{
			init_table();
		}
	}
	write_code(output, Cu, CODE_SIZE);
	write_code(output, 4095, CODE_SIZE);
}

void compress2(FILE *input, FILE *output)
{
	int C, N;
	std::string Cu;

	C = fgetc(input);
	if (C == EOF)
		return;
	N = fgetc(input);
	if (N == EOF)
		return;

	Cu += (char)C;
	while (N != EOF)
	{
		if (entry_map.find(Cu + (char)N) != entry_map.end())
		{
			Cu += (char)N;
			N = fgetc(input);
		}
		else
		{
			write_code(output, entry_map[Cu], CODE_SIZE);
			entry_map[Cu + (char)N] = entry_map.size() - 1;

			Cu = (char)N;
			N = fgetc(input);
		}

		if (entry_map.size() >= 4096)
		{
			init_map();
		}
	}
	write_code(output, entry_map[Cu], CODE_SIZE);
	write_code(output, 4095, CODE_SIZE);
}

void compress_enhanced(FILE *input, FILE *output)
{
	int code_size;
	int C, N;
	std::string Cu;

	C = fgetc(input);
	if (C == EOF)
		return;
	N = fgetc(input);
	if (N == EOF)
		return;

	Cu += (char)C;
	while (N != EOF)
	{
		if (entry_map.find(Cu + (char)N) != entry_map.end())
		{
			Cu += (char)N;
			N = fgetc(input);
		}
		else
		{
			int size = entry_map.size();
			if (size <= 511)
			{
				code_size = 9;
			}
			else if (size <= 1023)
			{
				code_size = 10;
			}
			else if (size <= 2047)
			{
				code_size = 11;
			}
			else
			{
				code_size = 12;
			}

			write_code(output, entry_map[Cu], code_size);

			entry_map[Cu + (char)N] = entry_map.size() - 1;

			Cu = (char)N;
			N = fgetc(input);
			if (entry_map.size() >= 4096)
			{
				init_map();
			}
		}
	}

	int size = entry_map.size();
	if (size <= 511)
	{
		code_size = 9;
	}
	else if (size <= 1023)
	{
		code_size = 10;
	}
	else if (size <= 2047)
	{
		code_size = 11;
	}
	else
	{
		code_size = 12;
	}
	write_code(output, entry_map[Cu], code_size);
	write_code(output, 0, code_size);
}

/*****************************************************************
 *
 * decompress() - decompress a compressed file to the orig. file
 *
 ****************************************************************/

void decompress(FILE *input, FILE *output)
{
	int CW, PW;
	CW = read_code(input, 12);
	PW = CW;
	print_string(output, CW);

	while ((CW = read_code(input, 12)) != 4095)
	{
		if (CW < table_idx)
		{
			print_string(output, CW);
			int c = first_char(CW);
			add_table(PW, c);
		}
		else
		{
			int c = first_char(PW);
			print_string(output, PW);
			fputc(c, output);
			add_table(PW, c);
		}
		PW = CW;

		if (table_idx == 4096)
		{
			init_table();
		}
	}
}

void decompress_enhanced(FILE *input, FILE *output)
{
	end_reading = false;
	int CW;
	std::string PW;
	int code_size;

	CW = read_code(input, 9);
	PW = reverse_map[CW];
	print_string(PW, output);

	while (!end_reading)
	{
		if (reverse_map_count + 1 <= 511)
		{
			code_size = 9;
		}
		else if (reverse_map_count + 1 <= 1023)
		{
			code_size = 10;
		}
		else if (reverse_map_count + 1 <= 2047)
		{
			code_size = 11;
		}
		else if (reverse_map_count + 1 <= 4095)
		{
			code_size = 12;
		}
		else
		{
			code_size = 9;
		}

		CW = read_code(input, code_size);

		if (reverse_map[CW].length() != 0)
		{
			print_string(reverse_map[CW], output);
			reverse_map[reverse_map_count++] = PW + reverse_map[CW][0];
		}
		else
		{
			if (end_reading)
				return;
			print_string(PW, output);
			fputc(PW[0], output);
			reverse_map[reverse_map_count++] = PW + PW[0];
		}
		PW = reverse_map[CW];

		if (reverse_map_count >= 4096)
		{
			init_reverse_map();
		}
	}
}

void decompress2(FILE *input, FILE *output)
{
	int CW;
	std::string PW;
	CW = read_code(input, 12);
	PW = reverse_map[CW];
	print_string(PW, output);

	while ((CW = read_code(input, 12)) != 4095)
	{
		if (reverse_map[CW].length() != 0)
		{
			print_string(reverse_map[CW], output);
			reverse_map[reverse_map_count++] = PW + reverse_map[CW][0];
		}
		else
		{
			print_string(PW, output);
			fputc(PW[0], output);
			reverse_map[reverse_map_count++] = PW + PW[0];
		}
		PW = reverse_map[CW];

		if (reverse_map_count >= 4096)
		{
			init_reverse_map();
		}
	}
}
