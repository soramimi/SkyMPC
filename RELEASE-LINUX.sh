
cd build
rm -fr SkyMPC
mkdir SkyMPC
rm SkyMPC.zip
cp Release/SkyMPC SkyMPC
cp ../SkyMPC_ja.qm SkyMPC
cp ../LinuxDesktop/SkyMPC.desktop SkyMPC
tar zcvf SkyMPC.tar.gz SkyMPC
