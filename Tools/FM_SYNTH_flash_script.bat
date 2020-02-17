:: Example script to use JLINK
ECHO OFF

:: Program parameters
set jlink_exe="C:\Program Files (x86)\SEGGER\JLink\JLink.exe"
set jlink_script_file=%1
set jlink_commander_script=%2
set jlink_device="STM32G070CB"
set jlink_if="SWD"
set jlink_speed=4000

:: Flash device
%jlink_exe% -JLinkScriptFile %jlink_script_file% -device %jlink_device% -if %jlink_if% -speed %jlink_speed% -autoconnect 1 -CommandFile %jlink_commander_script%
:: PAUSE