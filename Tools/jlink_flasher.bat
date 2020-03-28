:: Flasher script to use jlink
@ECHO OFF
SETLOCAL

:: Call parameter example
:: "C:\Program Files (x86)\SEGGER\JLink\jlink.exe" 
:: LPC54608J512 
:: "C:\Firmware\01_projects\00_TOOLS\test\bootstopper.bin" 
:: 0x00000000 

SET jlink_path=%~1
SET device_target=%~2
SET app_bin=%~3
SET app_addr=%~4

ECHO - JLINK PATH: %jlink_path%
ECHO - TARGET DEVICE: %device_target%
ECHO - APP: %app_bin% %app_addr%

:: Flash app
ECHO - FLASH APP
CALL :flash_bin %app_bin% %app_addr%

:: Finish script
EXIT /B %ERRORLEVEL%

:: Function to write bin
:flash_bin
:: Create tmp file with cmd to execute
SET bin_data=%1
SET bin_addr=%2
SET tmp_cmd_file=jlink_cmd.tmp
TYPE nul > %tmp_cmd_file%
ECHO r >> %tmp_cmd_file%
ECHO sleep 250 >> %tmp_cmd_file%
ECHO loadfile %bin_data% %bin_addr% >> %tmp_cmd_file%
ECHO r >> %tmp_cmd_file%
ECHO sleep 100 >> %tmp_cmd_file%
ECHO g >> %tmp_cmd_file%
ECHO exit >> %tmp_cmd_file%
:: Execute cmd with retry
SET /A retry_count=0
:flash_action_start
:: Execute flash command
"%jlink_path%" -Device %device_target% -If SWD -Speed 4000 -ExitOnError -CommandFile "%tmp_cmd_file%"
:: Process output to loop on error
IF /I "%ERRORLEVEL%" NEQ "0" (
    ECHO - FLASH RETRY %retry_count%
    IF /I "%retry_count%" LEQ "3" (
        SET /A retry_count=%retry_count% + 1
        ECHO - TRY TO FLASH AGAIN
        GOTO flash_action_start
    ) ELSE (
        ECHO - MAXIMUN RETRY COUNT ACHIEVE
    )
)
:: Delete tmp file
DEL %tmp_cmd_file%
EXIT /B 0
