@echo on
echo PLEASE EDIT THIS BAT FILE WITH CORRECT CONFIGURATION BEFORE RUN IT.
echo 请在运行本BAT文件前先修改为正确的设置



SET FONT_FILE=simsun.ttf

SET FONT_NAME=simsun

SET FONT_SIZE=9

FOR %%s IN (%FONT_SIZE%) DO CALL:ProcessFont %FONT_FILE%,%FONT_NAME%,%%s  

FOR /F "delims==" %%i IN ('dir code\u8g2_%FONT_NAME%_*.c /b') DO (CALL:FinishSourceCode %FONT_NAME%,%%~ni)


GOTO:EOF

:FinishSourceCode
	SET FN=%1
	SET CN=%2

	ECHO #include "%CN%.h" > code\temp.c
	TYPE code\%CN%.c >> code\temp.c
	DEL code\%CN%.c
	MOVE /y code\temp.c code\%CN%.c > NUL
	
	CALL :ToUpper %CN%,cnLowCase
        ECHO #ifndef _%cnLowCase%_H > code\temp.h
	ECHO #define _%cnLowCase%_H >> code\temp.h
	TYPE header1.txt >> code\temp.h
	ECHO extern const uint8_t %CN%[] U8G2_FONT_SECTION("%CN%"); >> code\temp.h
	TYPE header2.txt >> code\temp.h
	MOVE /y code\temp.h code\%cn%.h > NUL

GOTO:EOF

:ToUpper
	set upper=
	set "str=%~1%"
	for /f "skip=2 delims=" %%I in ('tree "\%str%"') do if not defined upper set "upper=%%~I"
	set "upper=%upper:~3%"
	set "%~2=%upper%"
GOTO:EOF

:ProcessFont
	SET FF=%1
	SET FN=%2
	SET FS=%3
	SET BDF=bdf/%FN%-%FS%.bdf
	echo Try to generate %BDF% ......
	otf2bdf -r 100 -p %FS% -o %BDF% font/%FF%
	CALL:CreateSourceCode %FN%,%FS%
GOTO:EOF

:CreateSourceCode
	SET FN=%1
	SET FS=%2
	SET BDF=bdf/%FN%-%FS%.bdf
	echo Try to generate source code code/u8g2_%FN%_%FS%_fntodgironchinese.c
	bdfconv -b 0 -f 1 -M map/fntodgironchinese.map -n u8g2_%FN%_%FS%_fntodgironchinese -o code/u8g2_%FN%_%FS%_fntodgironchinese.c %BDF% -d bdf/simsun-9.bdf
GOTO:EOF
