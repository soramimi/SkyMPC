cd build
cp -a Release/SkyMPC.app .
/opt/Qt5.6.0/5.6/clang_64/bin/macdeployqt SkyMPC.app
tar zcvf SkyMPC.zip SkyMPC.app
