# kernelinstall
Downloads, configures, compiles and installs the latest stable kernel from kernel.org

## Running it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```./kernel-installer```

## Compiling it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```make && sudo make install```
 * 4 run ```kernel-installer```

When compiling from source, be sure to have the required packages installed (make, gcc, gettext) as these are required. 


## Supported Distros:

 * Debian 13
 * Debian 12
 * MX Linux
 * Linux Mint LMDE
 * Linux Mint
 * Ubuntu
 * ElementaryOS
 * PopOS!

## Language Support

 * English
 * Spanish


This is a simple tool that fetches the latest stable kernel release from [kernel.org](https://kernel.org)
configures it for debian, compiles and installs it.

For now it only works on debian or distros that use debian as their upstream.


It needs testing on ubuntu, mint, elementary or some of those. Please create an issue if you are willing to test with the results.

Thanks!

-- Alexia.

## To Do 

 * Add more language support (French, Italian, German, Dutch, Swedish, Finnish and more)
 * Add support for rhel-based and arch-based distros

