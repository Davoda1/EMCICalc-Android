@echo off
rem Development tool only; the Android application launches from its APK icon.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0android-build.ps1" %*
exit /b %ERRORLEVEL%
