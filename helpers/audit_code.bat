@echo off
setlocal enabledelayedexpansion

set OUTPUT_FILE=code_audit.txt
echo Project Code Audit > %OUTPUT_FILE%
echo =================== >> %OUTPUT_FILE%
echo. >> %OUTPUT_FILE%

set /a totalLines=0
set /a totalFiles=0

REM Set keywords to track
set KEYWORDS=setup loop class struct enum WindowManager TFT Serial

for %%F in (*.cpp *.h *.ino *.c *.txt) do (
    set /a totalFiles+=1
    set /a fileLines=0

    for /f %%L in ('find /v /c "" ^< "%%F"') do (
        set /a fileLines=%%L
        set /a totalLines+=fileLines
    )

    echo File: %%F >> %OUTPUT_FILE%
    echo   Lines: !fileLines! >> %OUTPUT_FILE%

    for %%K in (%KEYWORDS%) do (
        for /f %%C in ('findstr /i /c:"%%K" "%%F" ^| find /c /v ""') do (
            echo     %%K: %%C >> %OUTPUT_FILE%
        )
    )

    echo. >> %OUTPUT_FILE%
)

echo =================== >> %OUTPUT_FILE%
echo Total files: %totalFiles% >> %OUTPUT_FILE%
echo Total lines: %totalLines% >> %OUTPUT_FILE%

echo Audit complete! See %OUTPUT_FILE%
pause
