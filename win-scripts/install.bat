if not exist "C:\%HOMEPATH%\.glogg" (
    mkdir "C:\%HOMEPATH%\.glogg"
)
xcopy *.bat "C:\%HOMEPATH%\.glogg\"

if not exist "C:\%HOMEPATH%\AppData\Roaming\glogg" (
    mkdir "C:\%HOMEPATH%\AppData\Roaming\glogg"
) else (
    xcopy "C:\%HOMEPATH%\AppData\Roaming\glogg\glogg.ini"  "C:\%HOMEPATH%\AppData\Roaming\glogg\glogg.ini.old"
)
xcopy glogg.ini "C:\%HOMEPATH%\AppData\Roaming\glogg"
start /b glogg.exe readme_win.txt

