@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ============================================
echo   DSK SFz Player - Sincronizar con GitHub
echo ============================================
echo.

echo [1/3] Comprobando y enviando cambios...
git status

:: Preguntar mensaje de commit
set /p MSG="Mensaje del commit (o enter para fecha): "
if "%MSG%"=="" set MSG=Update %date% %time%

:: Añadir todo y commit
git add -A
git commit -m "%MSG%"

echo.
echo [2/3] Sincronizando con el servidor (pull y push)...
git pull --rebase origin main
git push origin main

if %errorlevel% neq 0 (
    echo.
    echo ! ERROR: Hubo un problema con la sincronizacion.
    pause
    goto :fin
)

echo.
echo ============================================
echo   HECHO.
echo ============================================
git show --stat HEAD

:fin
echo.
set /p "OPENACT=Abrir GitHub Actions para compilar? (s/N): "
if /i "%OPENACT%"=="S" start "" "https://github.com/dskmusic/DSKSFzPlayer/actions"