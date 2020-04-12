:: Tool to extract info for app crash
@ECHO OFF
SETLOCAL

:: Defined tool paths
SET ARM_TOOL_PATH="C:/Firmware/arm_toochain/bin"
SET APP_EXE="C:/Firmware/git/MEGADRIVER/build/FM_SYNTH.elf"
SET ERROR_PC=%~1
SET ERROR_LK=%~2
REM SET ERROR_PC=0x80023EE
REM SET ERROR_LK=0x80082B5

:: Show parameters to use
ECHO.
ECHO - CRASHTOOL INPUTS:
ECHO.
ECHO EXEC: %APP_EXE%
ECHO PC: %ERROR_PC%
ECHO LK: %ERROR_LK%
ECHO.

:: Execute addre2line
ECHO - PROGRAM COUNTER: %ERROR_PC%
%ARM_TOOL_PATH%\arm-none-eabi-addr2line -e %APP_EXE% -s -f %ERROR_PC%
ECHO.
ECHO - LINK REGISTER: %ERROR_LK%
%ARM_TOOL_PATH%\arm-none-eabi-addr2line -e %APP_EXE% -s -f %ERROR_LK%
ECHO.
