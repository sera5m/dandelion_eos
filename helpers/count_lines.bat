@echo off
setlocal enabledelayedexpansion

set total=0

for %%F in (txt ino cpp c h) do (
    for /R %%G in (*.%%F) do (
        for /f %%H in ('find /v /c "" ^< "%%G"') do (
            set /a total+=%%H
        )
    )
)

echo ----------------------------------------
echo Total lines in .txt, .ino, .cpp, .c, .h files: !total!
echo ----------------------------------------

pause
