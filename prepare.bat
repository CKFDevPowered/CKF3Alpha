@echo off
echo Generating solution and project from template...
echo=

cd .\develop\
call .\prepare
cd %~dp0
echo=

cd .\src_ckf\Audio\develop
call .\prepare
cd %~dp0
echo=

echo Done. Press any key to continue...
pause>nul