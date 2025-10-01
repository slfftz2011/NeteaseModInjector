@echo off
chcp 65001
setlocal enabledelayedexpansion

windres resource.rc -o resource.o

:: 获取版本号
set /p version=<version.txt

:: 编译命令
echo 开始编译 ModInjector %version%...
g++ main.cpp resource.o -o "NeteaseModInjector_%version%.exe" -lshell32

:: 检查编译结果
if %errorlevel% equ 0 (
    echo 编译成功！
) else (
    echo 编译失败！
    exit /b 1
)

endlocal

pause