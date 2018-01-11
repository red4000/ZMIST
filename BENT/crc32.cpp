#include <windows.h>
#include "crc32.h"

DWORD CRC32(BYTE *ptr, DWORD size, DWORD crc) {
	crc = ~crc;

	while (size-- != 0) {
		DWORD i;
		DWORD c = *ptr++;

		for (i = 0; i < 8; i++) {
			if (((crc ^ c) & 1) != 0) {
				crc = (crc >> 1) ^ 0xEDB88320;
			} else {
				crc = (crc >> 1);
			}

			c >>= 1;
		}
	}

	return ~crc;
}

DWORD CRC32String(char *string) {
	return CRC32((BYTE*)string, (DWORD)strlen(string), 0);
}
