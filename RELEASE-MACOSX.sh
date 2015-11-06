cd build
cp -a Release/SkyMPC.app .
/opt/Qt5.5.1/5.5/clang_64/bin/macdeployqt SkyMPC.app
tar zcvf SkyMPC.zip SkyMPC.app
