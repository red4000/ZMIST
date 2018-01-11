# ZMIST
Updated z0mbie MISTFALL engine


One of the messiest projects I've ever developed, no documentation or help will be provided as this is solely for reference purposes and by all means needs to be completely redesigned and reimplemented. X64 support is nowhere near fixed/complete, but x86 Debug build can add 9 NOPs to RVA 1000h and reassemble itself without error.

Good luck!

See https://raw.githubusercontent.com/red4000/ZMIST/master/Debug/BentLog.txt

Loading xfent8.exe 402944(62600)
Phys size: 402944 / 0x62600
Virt size: 446464 / 0x6D000
section .text    at 01F0, 00001000..0004E173 vsize 0004D173 psize 0004E200
section .rdata   at 0218, 0004F000..0005C58C vsize 0000D58C psize 0000D600
section .data    at 0240, 0005D000..00068818 vsize 0000B818 psize 00003400
section .reloc   at 0268, 00069000..0006C288 vsize 00003288 psize 00003400
data descriptor 170: 00 / 00, 000562C0..0005BD9D size 5ADD IMAGE_DIRECTORY_ENTRY_EXPORT
data descriptor 178: 01 / 01, 0005BDA0..0005BE04 size 0064 IMAGE_DIRECTORY_ENTRY_IMPORT
data descriptor 198: 05 / 05, 00069000..0006C288 size 3288 IMAGE_DIRECTORY_ENTRY_BASERELOC
data descriptor 1A0: 06 / 06, 0004F1C0..0004F1F8 size 0038 IMAGE_DIRECTORY_ENTRY_DEBUG
data descriptor 1C0: 10 / 0A, 000554A0..000554E0 size 0040 IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG
data descriptor 1D0: 12 / 0C, 0004F000..0004F180 size 0180 IMAGE_DIRECTORY_ENTRY_IAT

...

Analyzed 578 blocks from signatures

...

82875 list entries, size 5304000(50EEC0)
New phys size: 402944 / 0x62600
New virt size: 450560 / 0x6E000 : 446464 / 0x6D000
Writing 402944(62600) bytes to xfent9.exe - C:\_new\a\GENT.tar\Debug\xfent9.exe
---BList---
OVA     |NVA     |DATA            |OPCODE                          |FLAGS
00000000|00000000|4D5A900003000000|(00000000..000000F8)            |FL_DATA,

...

00001001|00001001|0000000000000000|(00001001..00001020)            |FL_DATA,FL_EXECUTABLE,
00001020|00001020|                |                                |FL_LABEL,FL_CREF,
00000000|00001020|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001020|00001021|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001021|00001022|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001022|00001023|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001023|00001024|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001024|00001025|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001025|00001026|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001026|00001027|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001027|00001028|90              |nop                             |FL_OPCODE,FL_EXECUTABLE,
00001028|00001029|55              |push ebp                        |FL_OPCODE,FL_EXECUTABLE,
00001029|0000102A|89E5            |mov ebp, esp                    |FL_OPCODE,FL_EXECUTABLE,