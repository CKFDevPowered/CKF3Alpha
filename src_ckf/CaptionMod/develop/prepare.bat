@echo off
setlocal
    call :main
endlocal
goto :eof

:main
    echo Preparing Project CaptionMod...
    copy %~dp0template\** %~dp0
goto :eof