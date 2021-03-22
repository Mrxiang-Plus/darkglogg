@echo off
taskkill /im adb.exe /f
adb logcat -G 100M
adb logcat -c
IF %2=="" (
        START /B adb logcat >"%temp%\tmp.log"
        START /B %1\glogg.exe "%temp%\tmp.log"
) ELSE (
        for /F %%i in ('adb shell pgrep -f %2') do (
            for /F  %%d in (' adb shell ps -p %%i -o name=') do (
                IF "%%d"=="NAME" (
                    echo %%d
                ) ELSE (
                for /F  %%s in ('echo "%temp%\%%d_%%i.log"') do (
                    START /B adb logcat --pid=%%i>"%%s"
                    START /B %1\glogg.exe "%%s"
                )
            )
        )
    )
)
