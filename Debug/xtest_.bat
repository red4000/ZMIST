@echo off
fent -i fent.exe -o fent1.exe -oh fent1.h -l fent1.txt
fent1 -i fent1.exe -lh fent1.h -o fent2.exe -oh fent2.h -l fent2.txt -nd -nl
fent2 -i fent2.exe -lh fent2.h -o fent3.exe -oh fent3.h -l fent3.txt -nd -nl
fent3 -i fent3.exe -lh fent3.h -o fent4.exe -oh fent4.h -l fent4.txt -nd -nl
fent4 -i fent4.exe -lh fent4.h -o fent5.exe -oh fent5.h -l fent5.txt -nd -nl
fent5 -i fent5.exe -lh fent5.h -o fent6.exe -oh fent6.h -l fent6.txt -nd -nl
fent6 -i fent6.exe -lh fent6.h -o fent7.exe -oh fent7.h -l fent7.txt -nd
fent7 -i fent7.exe -lh fent7.h -o fent8.exe -oh fent8.h -l fent8.txt -nd
fent8 -i fent8.exe -lh fent8.h -o fent9.exe -oh fent9.h -l fent9.txt -nd
pause