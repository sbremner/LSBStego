/*
	Main program to be used for encoding/decoding files with steganography
*/

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>

#include "Stego.h"

#define ARGCMP(count, arg, ... ) \
	compare_args(count, arg, ##__VA_ARGS__)

bool compare_args(int count, char * arg, ...)
{
	va_list vl;
	va_start(vl, arg);
	
	char * str;

	for (int k = 0; k < count; k++)
	{
		str = va_arg(vl, char *);

		if (strcmp(arg, str) == 0) {
			return true;
		}
	}

	return false;
}

void writeFile(unsigned char * buf, unsigned long size, char * fName)
{
	FILE * fPtr = fopen(fName, "wb");

	fwrite(buf, 1, size, fPtr);

	fclose(fPtr);
}

unsigned long getFileSize(char * inFile)
{
	FILE * pFile;

	unsigned long fSize;

	pFile = fopen(inFile, "rb");

	fseek(pFile, 0, SEEK_END);
	fSize = ftell(pFile);

	fclose(pFile);

	return fSize;
}

//returns the size of the buffer read from inFile
unsigned long getBuffer(char * inFile, unsigned char * buffer)
{
	FILE * pFile;

	unsigned long fSize;

	size_t results;

	pFile = fopen(inFile, "rb");

	fseek(pFile, 0, SEEK_END);
	fSize = ftell(pFile);
	rewind(pFile);

	printf("Reading file {%s} to buffer...\n", inFile);
	results = fread(buffer, 1, fSize, pFile);
	printf("> Read success :: File size = %d Bytes\n", fSize);

	fclose(pFile); // done with input file - close it

	return fSize;
}

void packer(char * inFile, char * outFile, char * targetFile)
{
	unsigned char * inBuf = NULL;
	unsigned long inSize;

	unsigned char * targetBuf = NULL;
	unsigned long targetSize;

	// Initilize the space for our buffers
	inBuf = (unsigned char *)malloc(sizeof(unsigned char) * getFileSize(inFile));
	targetBuf = (unsigned char *)malloc(sizeof(unsigned char) * getFileSize(targetFile));

	if (inBuf == NULL || targetBuf == NULL)
	{
		fputs("Memory error during allocation\n", stderr);
		exit(1);
	}

	inSize = getBuffer(inFile, inBuf);
	targetSize = getBuffer(targetFile, targetBuf);

	if ((BITMAP_HEADER_LENGTH + ENCODED_LENGTH(inSize)) > targetSize)
	{
		fputs("Error - target file is not large enough to encode intput file\n", stderr);
		exit(2);
	}

	LSBHeader header;
	header.PayloadSize = inSize;
	header.PayloadType = 0;

	if (SetLSBHeader(targetBuf, targetSize, LSB_HEADER_OFFSET, &header) == FALSE)
	{
		fputs("Error - unable to set the LSB header\n", stderr);
		exit(3);
	}

	if (SetLSBPayload(targetBuf, targetSize, inBuf, inSize, LSB_PAYLOAD_OFFSET) == FALSE)
	{
		fputs("Error - unable to set the LSB payload\n", stderr);
		exit(4);
	}

	writeFile(targetBuf, targetSize, outFile);

	free(inBuf);
	free(targetBuf);
}

void unpack(char * targetFile, char * outFile)
{
	unsigned char * targetBuf = NULL;
	unsigned long targetSize;

	unsigned char * payload = NULL;
	LSBHeader * pHeader;

	targetBuf = (unsigned char *)malloc(sizeof(unsigned char) * getFileSize(targetFile));

	if (targetBuf == NULL)
	{
		fputs("Error - unable to allocate space for targetBuf\n", stderr);
	}

	targetSize = getBuffer(targetFile, targetBuf);

	pHeader = GetLSBHeader(targetBuf, targetSize, LSB_HEADER_OFFSET);

	if (pHeader == NULL || pHeader->PayloadSize == 0){
		fprintf(stderr, "Error - unable to read payload for encoded file.\n");
		return;
	}
	
	printf("Found file:\n");
	printf(" Size: 0x%X\n", pHeader->PayloadSize);
	printf(" Type: 0x%X\n", pHeader->PayloadType);

	payload = (unsigned char *)GetLSBPayload(targetBuf, targetSize, pHeader->PayloadSize, LSB_PAYLOAD_OFFSET);

	writeFile(payload, pHeader->PayloadSize, outFile);
}

void print_usage(char * exe)
{
	fputs(" Usage:\n\n", stdout);
	fprintf(stdout, "\t%s --encode <payload:filename> <output:filename> <bmp:filename>\n\n", exe);
	fprintf(stdout, "\t OR\n\n");
	fprintf(stdout, "\t%s --decode <bmp:filename> <output:filename>\n", exe);
}

int main(int argc, char * argv[])
{
	if (argc == 1 || ARGCMP(2, argv[1], "--help", "-h"))
	{
		print_usage(argv[0]);
		return 0;
	}

	if (ARGCMP(2, argv[1], "--decode", "-d"))
	{
		if (argc != 4){
			print_usage(argv[0]);
			return 0;
		}
		
		printf("Decoding File: {%s}\n", argv[2]);
		printf("Output File: {%s}\n", argv[3]);

		unpack(argv[2], argv[3]);
	}
	else if (ARGCMP(2, argv[1], "--encode", "-e"))
	{
		if (argc != 5){
			print_usage(argv[0]);
			return 0;
		}

		printf("Encoding File: {%s}\n", argv[2]);
		printf("Output File: {%s}\n", argv[3]);
		printf("Using bmp: {%s}\n\n", argv[4]);

		packer(argv[2], argv[3], argv[4]);
	}
	else
	{
		print_usage(argv[0]);
		return 0;
	}

	return 0;
}