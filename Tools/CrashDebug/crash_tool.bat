:: Tool to extract info for app crash
@ECHO OFF
SETLOCAL

:: CrashDebug: https://github.com/adamgreen/CrashDebug

:: Exe variables
::  GDB_TOOLPATH="C:\Firmware\arm_toochain\bin"
::  CRASH_DBG_APP="./CrashDebug.exe"
::  APP_FILE="../../build/MEGADRIVER.elf"
::  DUMP_FILE="./CrashDump.txt"
SET GDB_TOOLPATH=%~1
SET CRASH_DBG_APP=%~2
SET APP_FILE=%~3
SET DUMP_FILE=%~4

:: Execute tool
%GDB_TOOLPATH%\arm-none-eabi-gdb %APP_FILE% -ex "set target-charset ASCII" -ex "target remote | %CRASH_DBG_APP% --elf %APP_FILE% --dump %DUMP_FILE%"

PAUSE