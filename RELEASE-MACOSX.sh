cd build
rm -fr SkyMPC
mkdir SkyMPC
cp -a Release/SkyMPC.app SkyMPC
cp ../SkyMPC_ja.qm SkyMPC
/opt/Qt5.5.1/5.5/clang_64/bin/macdeployqt SkyMPC/SkyMPC.app

