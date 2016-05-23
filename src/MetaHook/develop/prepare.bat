@echo off
setlocal
    call :main
endlocal
goto :eof

:main
    echo Preparing Project ClientProxy...
    copy %~dp0template\** %~dp0
goto :eof