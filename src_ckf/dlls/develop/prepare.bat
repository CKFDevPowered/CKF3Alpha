@echo off
setlocal
    call :main
endlocal
goto :eof

:main
    echo Preparing Project mp...
    copy %~dp0template\** %~dp0
goto :eof