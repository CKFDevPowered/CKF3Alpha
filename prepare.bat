@echo off
setlocal
    call :main
endlocal
goto :eof

:main
    echo Generating solution and project from template...
    echo=
    
    for /F %%i in (prepare.list) do (
        call %~dp0%%i
        echo=
    )

    echo Done. Press any key to continue...
    pause > nul
goto :eof