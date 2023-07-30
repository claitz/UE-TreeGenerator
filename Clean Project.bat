@echo off
setlocal
:PROMPT
set /p var=Are you sure you want to clean the project? [Y/n]: || SET "var=Y"

if /I not %var%== Y exit

del /q/s *".sln"

rmdir /q/s ".vs"
rmdir /q/s "Build"
rmdir /q/s "Binaries"
rmdir /q/s "DerivedDataCache"
rmdir /q/s "Intermediate"
rmdir /q/s "Platforms"
rmdir /q/s "Saved"

set /p exit=Finished. Press enter to exit. || SET "exit=Y"
if /I %exit%== Y exit
endlocal