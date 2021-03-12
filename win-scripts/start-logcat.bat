@echo off
taskkill /im adb.exe /f
adb logcat -c
adb logcat -G 256M 
::for /F %%i in ('adb shell pgrep -f com.android.camera') do ( set pid=%%i)
::adb logcat --pid=%pid%>"%HOMEPATH%\glogg\tmp.log"
START /B adb logcat >"%temp%\tmp.log"
start /b %1\glogg.exe "%temp%\tmp.log"