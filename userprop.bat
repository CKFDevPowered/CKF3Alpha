@echo off
if "%1" == "" (
    call :print-usage
    goto :eof
)
setlocal
    call :main %1
endlocal
goto :eof

:main
    if "%1" == "backup" (
        echo Backing up user prop.
        copy %~dp0develop\user.props %~dp0develop\user.props.bak
    ) else if "%1" == "restore" (
        echo Restoring user prop.
        copy %~dp0develop\user.props.bak %~dp0develop\user.props
    ) else (
        call :print-usage
        goto :eof
    )
    echo=

    echo Done. Press any key to continue...
    pause > nul
goto :eof

:print-usage
    echo Usage: userprop [backup^|restore]
goto :eof