#ifndef CRC32_H
#define CRC32_H

DWORD CRC32(BYTE *ptr, size_t size, DWORD crc);
DWORD CRC32String(char *string);

#endif
