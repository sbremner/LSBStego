#include "Utils.h"

void __cdecl memset(void * pMem, DWORD dwLength, UCHAR value)
{
	unsigned char * pData = (unsigned char *)pMem;

	for (unsigned int __iter__ = 0; __iter__ < dwLength; __iter__++)
	{
		pData[__iter__] = value;
	}
}