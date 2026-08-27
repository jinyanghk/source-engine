
sudo apt update
sudo apt install -y python3 python3-pip python3-venv


python3 -m venv aqt-env
source aqt-env/bin/activate
pip install aqtinstall

aqt install-qt linux desktop 6.10.3 linux_gcc_64 -m qtcharts qtmultimedia




sudo apt install -y qt6-base-dev qt6-tools-dev libqt6svg6-dev

sudo apt install -y qt6-svg-dev