set path=%path%;c:\Program Files (x86)\WiX Toolset v3.7\bin;c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin
copy ..\bin\Release\*.exe .
signtool sign /f ..\..\keys\exesigning\LucianIonSPC.pfx /p xxxxxx /t http://timestamp.verisign.com/scripts/timstamp.dll *.exe
candle WinSndRmt.wxs
light WinSndRmt.wixobj
del *.exe *.wixpdb *.wixobj
signtool sign /f ..\..\keys\exesigning\LucianIonSPC.pfx /p xxxxxx /t http://timestamp.verisign.com/scripts/timstamp.dll *.msi
move WinSndRmt.msi ..\bin\Release\
pause