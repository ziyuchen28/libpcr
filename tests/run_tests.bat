 
@echo off
setlocal

ctest --test-dir build --output-on-failure "%*"

if %errorlevel% neq 0 exit /b %errorlevel%
