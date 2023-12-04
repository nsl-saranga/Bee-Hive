@echo off
for /f "eol= delims=" %%i in ('type "secrets.env"') do set "%%i"
set /P e="Enter the run environment: "
call "%%USERPROFILE%%\.platformio\penv\Scripts\pio.exe" run -e %%e%%
