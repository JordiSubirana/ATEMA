@echo off

if "%1" == "" goto platformRequired

"./premake/premake5.exe" --file="./scripts/premake5.lua" %1
goto :eof

:platformRequired
	echo Error : unspecified target platform (%0 ^<TargetPlatform^>)