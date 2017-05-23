## Setting up environment

```
$ cd myfolder
$ sudo apt-get install libtool-bin
$ git clone --recursive https://github.com/pfalcon/esp-open-sdk.git
$ make
``` 
## Setting environmental path for xtensa toolchain

```
$ sudo nano ~/.profile
```
Add `export PATH=myfolder/esp-open-sdk/xtensa-lx106-elf/bin:$PATH`

Logout after finish editing file

## Pulling updates

```
$ make clean
$ git pull
$ git submodule sync
$ git submodule update --init
```

## Compile AT project

* Create a new folder.
* Copy **at** folder from sdk/examples to the new folder.
* Copy all the files/folders from sdk/ (except for examples/) to the new folder.
* Run gen_misc.sh from **at/** to generate bin files.
