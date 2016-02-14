
cd build
rm -fr SkyMPC
mkdir SkyMPC
rm SkyMPC-raspi.tar.gz SkyMPC
cp Raspberry/SkyMPC SkyMPC
cp ../SkyMPC_ja.qm SkyMPC
cp ../SkyMPC.svg SkyMPC
cp ../LinuxDesktop/SkyMPC.desktop SkyMPC
tar zcvf SkyMPC-raspi.tar.gz SkyMPC
