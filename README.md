# kernelinstall
Downloads, configures, compiles and installs the latest stable kernel from kernel.org

## Running it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```./kernel-install```

## Compiling it:

 * 1 Clone the repository
 * 2 ```cd kernelinstall```
 * 3 ```make && sudo make install```
 * 4 run ```kernelinstall```

## Supported Distros:

 * Debian
 * MX Linux

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

Support for more distros by order of relevance. There are plans to cover Ubuntu, Linux Mint, Fedora, ElementaryOS and Arch Linux
