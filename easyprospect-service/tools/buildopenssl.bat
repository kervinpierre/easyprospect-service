REM 1. Use Strawberry Perl, and NOT ActiveState Perl
REM 2. See https://gist.github.com/terrillmoore/995421ea6171a9aa50552f6aa4be0998
REM 3. Run vcvars64.bat even under Deverloper Command Line

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
perl Configure debug-VC-WIN64A --prefix=c:\OpenSSL\debug-vc-win64a -openssldir=c:\OpenSSL\SSL no-shared

nmake
nmake test
nmake install