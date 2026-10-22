@echo off
setlocal
cd /d "%~dp0"
python -B "%~dp0..\xport\tools\xport.py" --project "%~dp0." record %*
exit /b %errorlevel%
