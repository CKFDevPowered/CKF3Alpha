@echo off
setlocal
    call :main
endlocal
goto :eof

:main
    echo Preparing Project Audio...
    copy %~dp0template\** %~dp0
goto :eof