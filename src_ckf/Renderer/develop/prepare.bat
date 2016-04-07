@echo off
setlocal
    call :main
endlocal
goto :eof

:main
    echo Preparing Project Renderer...
    copy %~dp0template\** %~dp0
goto :eof