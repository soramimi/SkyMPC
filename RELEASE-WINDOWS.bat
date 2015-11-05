cd build
rmdir /s /q SkyMPC
mkdir SkyMPC
del SkyMPC.zip
copy Release\SkyMPC.exe SkyMPC
copy ..\SkyMPC_ja.qm SkyMPC
7za a SkyMPC.zip SkyMPC
