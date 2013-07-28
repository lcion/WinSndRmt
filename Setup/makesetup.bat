set path=%path%;c:\Program Files (x86)\WiX Toolset v3.7\bin
copy ..\bin\Release\*.exe .
candle WinSndRmt.wxs
light WinSndRmt.wixobj
del *.exe *.wixpdb *.wixobj
pause