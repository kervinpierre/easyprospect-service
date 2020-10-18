REM https://v8.dev/docs/source-code
REM https://v8.dev/docs/build
REM https://v8.dev/docs/build-gn
REM https://v8.dev/docs/embed

REM Build V8 on windows in an already downloaded directory
REM using GN.
REM
REM Build and specify and output directory
REM E.g. buildv801.bat -b=out\x64_3.debug
REM
REM For new tags...
REM git pull origin
REM
REM Switch branches...
REM git checkout refs/tags/7.9.317.31

set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set V8DIR=c:\v8\src\
set DEPOTTOOLSDIR=i:/depot_tools/
set BUILDNAME=x642
set BUILDCONF=debug
set BUILDDIR=%BUILDNAME%.%BUILDCONF%
set BUILDPATH=out\%BUILDDIR%

set "PATH=%DEPOTTOOLSDIR%;%PATH%"

set DOCLEAN=0

set "c="     REM Clean
set "b="     REM Build directory

:initial
if "%1"=="" goto done
echo              %1
set aux=%1
if "%aux:~0,1%"=="-" (
   set nome=%aux:~1,250%
) else (
   set "%nome%=%1"
   set nome=
)
shift
goto initial
:done

if not "%b%" == "" ( set "BUILDPATH=%b%" )
if not "%c%" == "" ( set DOCLEAN=1 )

set "b="
set "c="

if not "%DOCLEAN%" equ "0" ( set DOCLEAN=1 )

set "PKGPATH=%BUILDPATH%\package"

pushd %V8DIR%

call gclient sync -D

pushd v8

set GNARGS="is_clang=false is_debug=true use_goma=false target_cpu=""x64"" v8_enable_backtrace=true v8_enable_slow_dchecks=true v8_optimized_debug=false v8_enable_backtrace=true is_component_build=false v8_static_library=true v8_use_external_startup_data=false use_custom_libcxx=false use_custom_libcxx_for_host=false use_lld=false v8_monolithic=true"

call gn gen %BUILDPATH%  --ide=vs  --args=%GNARGS%
ninja -C %BUILDPATH% v8_monolith

@echo on

if errorlevel 1 (
   echo Failure Reason Given is %errorlevel%
   exit /b %errorlevel%
)

del /f /s /q %PKGPATH% 1>nul
rd /s /q %PKGPATH%
waitfor /t 5 pause 2>nul
if exist %PKGPATH% rd /s /q %PKGPATH%

mkdir %PKGPATH% %PKGPATH%\v8\ %PKGPATH%\v8\%BUILDCONF% %PKGPATH%\v8\%BUILDCONF%\libs\ %PKGPATH%\v8\%BUILDCONF%\include\

robocopy include %PKGPATH%\v8\%BUILDCONF%\include\ /E
copy %BUILDPATH%\obj\v8_monolith.lib %PKGPATH%\v8\%BUILDCONF%\libs\

popd
popd 