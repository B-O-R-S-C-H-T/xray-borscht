@echo off

chcp 1251 > nul
setlocal EnableDelayedExpansion

if "%~1"=="" (
    echo Не указан исходный каталог!
    echo Использование: %0 ^<исходный_каталог^> ^<целевой_каталог^>
    pause
    exit /b 1
)
if "%~2"=="" (
    echo Не указан целевой каталог!
    echo Использование: %0 ^<исходный_каталог^> ^<целевой_каталог^>
    pause
    exit /b 1
)

set "source_dir=%~1"
set "dest_dir=%~2"

if not exist "%source_dir%\" (
    echo Исходный каталог "%source_dir%" не существует!
    pause
    exit /b 1
)

if not exist "%dest_dir%\" (
    echo Создание целевого каталога "%dest_dir%"
    mkdir "%dest_dir%"
    if errorlevel 1 (
        echo Не удалось создать целевой каталог "%dest_dir%".
        pause
        exit /b 1
    )
)

set "file_count=0"

for %%F in ("%source_dir%\*.dll") do (
    echo Копирование %%F в %dest_dir%
    copy /Y "%%F" "%dest_dir%\"
    if errorlevel 1 (
        echo Не удалось скопировать %%F
    ) else (
        set /a file_count+=1
    )
)

for %%F in ("%source_dir%\*.exe") do (
    echo Копирование %%F в %dest_dir%
    copy /Y "%%F" "%dest_dir%\"
    if errorlevel 1 (
        echo Не удалось скопировать %%F
    ) else (
        set /a file_count+=1
    )
)

if !file_count! equ 0 (
    echo Не найдены .dll или .exe в "%source_dir%"
) else (
    echo Успешно скопировано файлов: !file_count! в: "%dest_dir%"
)

pause
endlocal
exit /b 0
