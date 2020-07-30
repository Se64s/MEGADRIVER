:: Flasher script to use jlink
@ECHO OFF
SETLOCAL

:: Call parameter example
:: "C:\Program Files (x86)\SEGGER\JLink\jlink.exe" 
:: LPC54608J512 

SET jlink_path=%~1
SET device_target=%~2

ECHO - JLINK PATH: %jlink_path%
ECHO - TARGET DEVICE: %device_target%

:: Flash app
ECHO - FLASH APP
CALL :flash_erase

:: Finish script
EXIT /B %ERRORLEVEL%

:: Function to write bin
:flash_erase
:: Create tmp file with cmd to execute
SET tmp_cmd_file=jlink_cmd.tmp
TYPE nul > %tmp_cmd_file%
ECHO r >> %tmp_cmd_file%
ECHO sleep 250 >> %tmp_cmd_file%
ECHO erase >> %tmp_cmd_file%
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
