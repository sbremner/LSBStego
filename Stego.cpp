/*
    Implementation of Stego functions used to extract LSB encoded bitmaps
*/

#include "Stego.h"
#include "Utils.h"

// General formula to encode into pBuffer (called from SetLSBHeader and SetLSBPayload)
BOOL EncodeLSB(unsigned char * pBuffer, DWORD dwBufferLength, DWORD dwOffset, void * pData, DWORD dwDataLength)
{
	if (dwBufferLength < (dwOffset + ENCODED_LENGTH(dwDataLength)))
	{
		return FALSE; // Error - not enough room to encode the payload
	}

	unsigned char * pRaw = (unsigned char *)pData;

	unsigned int index = 0;
	for (unsigned int i = dwOffset; i < (dwOffset + ENCODED_LENGTH(dwDataLength)); i++, index++)
	{
		ZERO_LSB(pBuffer[i]); // Zero out the last bit so we can encode in it
		pBuffer[i] |= ((pRaw[index / 8] >> (index % 8)) & 0x1); // Encode LSB
	}

	return TRUE;
}

// General formula to decode into raw data returned as void * (called from GetLSBHeader and GetLSBPayload)
void * DecodeLSB(unsigned char * pBuffer, DWORD dwBufferLength, DWORD dwOffset, DWORD dwPayloadSize)
{
	if (dwBufferLength < (dwOffset + ENCODED_LENGTH(dwPayloadSize)))
	{
		return NULL; // Requesting too large of a payload from the pBuffer
	}

	// Allocate room for our payload (we will only be getting exactly this much out of the pBuffer)
	unsigned char * pData = (unsigned char *)malloc(dwPayloadSize);
	memset(pData, dwPayloadSize, 0);

	unsigned int index = 0;
	for (unsigned int i = dwOffset; i < dwBufferLength && (index / 8) < dwPayloadSize; i++, index++)
	{
		pData[index / 8] |= (LSB(pBuffer[i]) << (index % 8));
	}

	return (void *)pData;
}

LSBHeader * GetLSBHeader(unsigned char * buffer, DWORD dwBufferLength, DWORD dwHeaderOffset)
{
	return (LSBHeader *)DecodeLSB(buffer, dwBufferLength, dwHeaderOffset, sizeof(LSBHeader));
}

void * GetLSBPayload(unsigned char * buffer, DWORD dwBufferLength, DWORD dwPayloadLength, DWORD dwPayloadOffset)
{
	return DecodeLSB(buffer, dwBufferLength, dwPayloadOffset, dwPayloadLength);
}

// unsigned char * buffer :: The LSB Header will be encoded into this buffer using LSB encoding
BOOL SetLSBHeader(unsigned char * buffer, DWORD dwBufferLength, DWORD dwHeaderOffset, LSBHeader * pLSBHeader)
{
	return EncodeLSB(buffer, dwBufferLength, dwHeaderOffset, (void *)pLSBHeader, sizeof(LSBHeader));
}

// unsigned char * buffer :: The payload will be encoded into this buffer using LSB encoding
BOOL SetLSBPayload(unsigned char * buffer, DWORD dwBufferLength, unsigned char * payload, DWORD dwPayloadLength, DWORD dwPayloadOffset)
{
	return EncodeLSB(buffer, dwBufferLength, dwPayloadOffset, (void *)payload, dwPayloadLength);
}