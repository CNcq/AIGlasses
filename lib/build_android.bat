@echo off
REM AI Glasses Android 库构建脚本 (Windows 版本)

setlocal enabledelayedexpansion

REM 配置变量
if "%ANDROID_NDK%"=="" (
    echo [ERROR] 请设置 ANDROID_NDK 环境变量
    echo 示例：set ANDROID_NDK=C:\Android\android-ndk-r25b
    exit /b 1
)

set BUILD_DIR=build_android
set INSTALL_DIR=install
set MIN_API=21

REM 支持的 ABI
set ABIS=arm64-v8a armeabi-v7a x86 x86_64

echo ========================================
echo AI Glasses Android 库构建
echo ========================================
echo NDK: %ANDROID_NDK%
echo 构建目录：%BUILD_DIR%
echo 安装目录：%INSTALL_DIR%
echo ========================================

REM 检查 NDK
if not exist "%ANDROID_NDK%" (
    echo [ERROR] NDK 路径不存在：%ANDROID_NDK%
    exit /b 1
)

REM 清理
:clean
echo [INFO] 清理构建目录...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%INSTALL_DIR%" rmdir /s /q "%INSTALL_DIR%"
if exist "dist" rmdir /s /q "dist"

REM 构建函数
:build
for %%a in (%ABIS%) do (
    call :build_abi %%a
)
goto :package

:build_abi
set ABI=%1
echo [INFO] 构建 ABI: %ABI%

mkdir "%BUILD_DIR%\%ABI%" 2>nul
cd "%BUILD_DIR%\%ABI%"

cmake ../.. ^
    -G "MinGW Makefiles" ^
    -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK%\build\cmake\android.toolchain.cmake" ^
    -DANDROID_ABI="%ABI%" ^
    -DANDROID_PLATFORM="android-%MIN_API%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS=ON ^
    -DBUILD_ANDROID_JNI=ON ^
    -DBUILD_TESTS=OFF ^
    -DCMAKE_INSTALL_PREFIX="../../%INSTALL_DIR%\%ABI%"

if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake 配置失败
    cd ../..
    exit /b 1
)

mingw32-make -j4
if %ERRORLEVEL% neq 0 (
    echo [ERROR] 编译失败
    cd ../..
    exit /b 1
)

mingw32-make install
if %ERRORLEVEL% neq 0 (
    echo [ERROR] 安装失败
    cd ../..
    exit /b 1
)

cd ../..
echo [INFO] %ABI% 构建成功
goto :eof

REM 打包函数
:package
echo [INFO] 打包库文件...

if exist "dist" rmdir /s /q "dist"
mkdir "dist"

REM 复制头文件
xcopy /E /I /Y "%INSTALL_DIR%\arm64-v8a\include" "dist\include"

REM 复制 so 文件
for %%a in (%ABIS%) do (
    if exist "%INSTALL_DIR%\%%a\lib" (
        mkdir "dist\libs\%%a"
        xcopy /Y "%INSTALL_DIR%\%%a\lib\*.so" "dist\libs\%%a\"
    )
)

echo [INFO] 打包完成：dist
goto :end

REM 主程序
:main
if "%1"=="" goto :all
if "%1"=="clean" goto :clean
if "%1"=="build" goto :build
if "%1"=="package" goto :package
if "%1"=="all" goto :all

echo 用法：%0 [选项]
echo 选项：clean, build, package, all
exit /b 1

:all
call :clean
call :build
call :package

:end
echo ========================================
echo 构建完成!
echo ========================================
endlocal
