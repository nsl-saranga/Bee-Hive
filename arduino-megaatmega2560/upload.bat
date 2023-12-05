@echo off
set /P e="Enter the run environment: "
call "%%USERPROFILE%%\.platformio\penv\Scripts\pio.exe" run -e %%e%%
