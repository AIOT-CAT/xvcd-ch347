# xvcd-ch347
CH347 Xilinx Virtual Cable
 xvcd_ch347 of XVC (Xilinx Virtual Cable) protocol based on xvcd (https://github.com/tmbinc/xvcd)

 1. **compile**
 
	## Windows
		1、mkdir build
		2、cd build 
		3、cmake ../
		4、ninja.exe
	Regarding CH347DLLA64.DLL, you can obtain it from here : https://www.wch.cn/downloads/CH341PAR_ZIP.html
	## Linux
		1、mkdir build
		2、cd build 
		3、cmake ../
		4、make
    Using the libusb(https://github.com/libusb/libusb.git) library installed under the system, if on the PC side, you can execute：
    
	sudo apt get install libusb-1.0-0-dev
	sudo apt-get install libusb-dev

 
 2. **About clock**
 
    Support : KHZ(468.75), KHZ(937.5), MHZ(1.875), MHZ(3.75), MHZ(7.5), MHZ(15), MHZ(30), MHZ(60)
    If you need to set a 10MHz clock, choose 15MHz or 7.5MHz, and modify the DEFAULT_JTAG-SPEED variable in xvcd_win. c to correspond to the clock, such as 150000000


 3. If you have a better way to accelerate the speed of this xvcd, please communicate with me for research, or if there are other integrated methods such as Vivado, we can also communicate together. If there are results, we can open them up again to serve more developers, **oidcatiot@163.com**


 4. **TODO:**
 
    Operation in remote server mode...
