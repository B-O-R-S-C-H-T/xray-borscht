@echo off
:: апдейт без луа и багтрапа в боржч
call deployer.cmd "W:\stk-src\borscht\intermediate\x64-Debug" "X:\bin_x64_dbg"

:: полный релиз из борща в чистое небо
call deployer.cmd "X:\bin_x64_dbg" "C:\test\csky\bin_x64"
pause