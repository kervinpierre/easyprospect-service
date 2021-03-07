
set "soldir=%1"
set "outdir=%2"

pushd %soldir%
%outdir%\incbin.exe include\easyprospect-data\ep-incbin-01.in.h -o include\easyprospect-data\ep-incbin-01.h
popd
