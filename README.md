# xvcd-ch347

CH347 Xilinx Virtual Cable

xvcd_ch347 of XVC (Xilinx Virtual Cable) protocol based on xvcd (https://github.com/tmbinc/xvcd)

## Compile

### Windows

First, ensure you have Git and either Visual Studio (requires a license for commercial use) or Build Tools for Visual Studio (free from Microsoft) installed.

Open **Developer PowerShell for Visual Studio** and run the following commands:

```powershell
git clone https://github.com/AIOT-CAT/xvcd-ch347.git
New-Item -Path 'xvcd-ch347\build' -ItemType Directory
Set-Location 'xvcd-ch347\build'
cmake ..
cmake --build . --config Release
```

To obtain `CH347DLLA64.DLL`, download it from: [https://www.wch.cn/downloads/CH341PAR\_ZIP.html](https://www.wch.cn/downloads/CH341PAR_ZIP.html).

### Linux

First install dependences:

```bash
sudo apt-get update
sudo apt-get install libusb-1.0-0-dev
# This installs libusb 0.0.1, no need to install unless you clear what you are doing.
# sudo apt-get install libusb-dev
sudo apt install build-essentials cmake
```

or you can install libusb manually from: https://github.com/libusb/libusb.git

```bash
git clone https://github.com/AIOT-CAT/xvcd-ch347.git
mkdir -p xvcd-ch347/build
cd xvcd-ch347/build
cmake ..
make
```

## FAQ

1. **About clock**

   Support : `KHZ(468.75)`, `KHZ(937.5)`, `MHZ(1.875)`, `MHZ(3.75)`, `MHZ(7.5)`, `MHZ(15)`, `MHZ(30)`, `MHZ(60)`.

   If you need to set a 10MHz clock, choose 15MHz or 7.5MHz, and modify the `DEFAULT_JTAG-SPEED` variable in `xvcd_win.c` to correspond to the clock, such as `150000000`.

   If you have a better way to accelerate the speed of this xvcd, please communicate with me for research, or if there are other integrated methods such as Vivado, we can also communicate together. If there are results, we can open them up again to serve more developers, **oidcatiot@163.com**


2. **TODO**

   Operation in remote server mode...
