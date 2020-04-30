:: Tool to extract info for app crash
@ECHO OFF
SETLOCAL

:: Call parameter example
:: "C:/Firmware/arm_toochain/bin"
:: "C:/Firmware/git/MEGADRIVER/build/FM_SYNTH.elf"
:: 0x80023EE
:: 0x80082B5

:: Defined tool paths
SET ARM_TOOL_PATH=%~1
SET APP_EXE=%~2
SET ERROR_PC=%~3
SET ERROR_LK=%~4

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
