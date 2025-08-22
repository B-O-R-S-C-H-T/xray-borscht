@echo off
:: апдейт без луа и багтрапа в боржч
call deployer.cmd "W:\stk-src\borscht\intermediate\x64-Release" "X:\bin_x64"

:: полный релиз из борща в чистое небо
call deployer.cmd "X:\bin_x64" "C:\test\csky\bin_x64"
pause