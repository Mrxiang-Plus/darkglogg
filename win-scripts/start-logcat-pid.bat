@echo off
taskkill /im adb.exe /f
adb logcat -G 100M 
adb logcat -c
IF "%2"=="" (
    echo ======
    START /B adb logcat >"%temp%\tmp.log"
    start /b %1\glogg.exe "%temp%\tmp.log"

) ELSE (
    for /F %%pid in ('adb shell pgrep -f %2') do ( 
        set str=('adb shell ps -p %pid% -o name')
        IF "%str%"=="" (
            echo %pid%
        ) ELSE (
            set str=%str:=%
            set name=%temp%\%str%_%pid%.log
            adb logcat --pid=%pid% > %name%
            start /b %1\glogg.exe %name%
        )
    )
)
