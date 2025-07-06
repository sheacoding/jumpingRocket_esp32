@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================================
echo 🚀 蹦跳小火箭 - 开发板环境切换工具 (Windows)
echo ============================================================

REM 检查Python是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo ❌ 错误: 未找到Python
    echo 请先安装Python: https://www.python.org/downloads/
    pause
    exit /b 1
)

REM 检查PlatformIO是否安装
pio --version >nul 2>&1
if errorlevel 1 (
    echo ❌ 错误: 未找到PlatformIO
    echo 请先安装PlatformIO: https://platformio.org/install
    pause
    exit /b 1
)

REM 如果没有参数，显示帮助
if "%1"=="" (
    echo 用法:
    echo   %0 list                    - 列出支持的开发板
    echo   %0 esp32c3                 - 切换到ESP32-C3
    echo   %0 esp32                   - 切换到ESP32
    echo   %0 esp32c3 build           - 切换并编译
    echo   %0 esp32 build upload      - 切换、编译并上传
    echo.
    echo 按任意键查看支持的开发板...
    pause >nul
    python switch_board.py --list
    pause
    exit /b 0
)

REM 处理参数
set BOARD=%1
set ACTION1=%2
set ACTION2=%3

REM 构建Python命令
set PYTHON_CMD=python switch_board.py

if "%BOARD%"=="list" (
    set PYTHON_CMD=!PYTHON_CMD! --list
) else (
    set PYTHON_CMD=!PYTHON_CMD! %BOARD%
    
    if "%ACTION1%"=="build" (
        set PYTHON_CMD=!PYTHON_CMD! --build
    )
    
    if "%ACTION1%"=="upload" (
        set PYTHON_CMD=!PYTHON_CMD! --upload
    )
    
    if "%ACTION1%"=="monitor" (
        set PYTHON_CMD=!PYTHON_CMD! --monitor
    )
    
    if "%ACTION2%"=="build" (
        set PYTHON_CMD=!PYTHON_CMD! --build
    )
    
    if "%ACTION2%"=="upload" (
        set PYTHON_CMD=!PYTHON_CMD! --upload
    )
    
    if "%ACTION2%"=="monitor" (
        set PYTHON_CMD=!PYTHON_CMD! --monitor
    )
)

REM 执行Python脚本
echo 执行命令: !PYTHON_CMD!
echo.
!PYTHON_CMD!

REM 检查执行结果
if errorlevel 1 (
    echo.
    echo ❌ 操作失败
    pause
    exit /b 1
) else (
    echo.
    echo ✅ 操作完成
    if "%ACTION2%"=="monitor" (
        REM 如果是监控模式，不要暂停
        exit /b 0
    ) else (
        echo 按任意键继续...
        pause >nul
        exit /b 0
    )
)
