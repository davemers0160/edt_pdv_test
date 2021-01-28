# EDT PDV Test Code

Code to test the basics of setting up the EDt PDV driver API and lining to the functions.

This code also uses the sierra-olympic, wind_protocol and fip_protocols to build control packets for the Sierra Olympic cameras.

The link here is a Doxygen link to the [Documentation](https://davemers0160.github.io/edt_pdv_test/html/index.html) of the classes to build all of the control packets.


## Ubuntu 18.04
```
sudo apt-get update
```

check these to make sure that the right header version is used use the link below to verify the closet match between the kernel headers and the linux source https://packages.ubuntu.com/bionic/devel/
```
sudo apt-get install -y --fix-missing linux-headers-$(uname -r)
sudo apt-get install -y --fix-missing linux-source-5.3.0
```

install the required packages
```
sudo apt-get install -y --fix-missing flex bison libqt4-dev make build-essential software-properties-common unzip libtiff5-dev libncurses5-dev kmod pciutils
```

get the current EDT pdv driver
```
wget https://edt.com/wp-content/uploads/2019/07/EDTpdv_lnx_5.5.8.9.run
chmod +x EDTpdv_lnx_5.5.8.9.run
```

copy the source files into the right location
```
cd /usr/src/linux-source-5.3.0
sudo tar -jxvf linux-source-5.3.0.tar.bz2
sudo cp -a /usr/src/linux-source-5.3.0/linux-source-5.3.0/. /usr/src/linux-hwe-5.4-headers-5.4.0-52/
```

