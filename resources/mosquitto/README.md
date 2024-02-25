# How to cross compile mosquitto

## Install cross compile toolchain for your target

Use crosstool-ng toolchain generator to compile the target toolchain and sysroot.
Here the host is ArchLinux x86_64 and the target aarch64 (64-bit ARM) such as 
the Odroid C2/C4/M1.

```
mkdir cross; cd cross
ct-ng list-samples
ct-ng aarch64-unknown-linux-gnu
ct-ng menuconfig
```
Select the same gcc and gdb major versions which are also installed on the target.
For Archlinux Arm this is currently gcc 12.1 (select gcc 12.3 and gdb 12.1 in ct-ng).
Build and install the toolchain with `ct-ng build`. The final toolchain is in the 
`$HOME/x-tools/aarch64-unknown-linux-gnu` folder.

## Prepare mosquitto source

Download and extract the mosquitto source files. Copy the cross compile patch
and the toolchain cmake file into the source folder and apply the patch.

```
cd froniusd/resources/mosquitto
wget https://mosquitto.org/files/source/mosquitto-2.0.18.tar.gz
tar xvzf mosquitto-2.0.18.tar.gz
cp cross_compile-2.0.18.patch toolchain.cmake mosquitto-2.0.18
cd mosquitto-2.0.18
patch -Np1 -i cross_compile-2.0.18.patch
```

## Compile and install into sysroot

Run cmake followed by make and make install. Install missing packages 
into host if there are compile errors due to missing headers. `uthash` 
headers need to be installed manually into sysroot.

```
cmake CMakeLists.txt
sudo pacman -S uthash
cd /usr/include
sudo cp utarray.h uthash.h utlist.h utringbuffer.h utstack.h utstring.h \
  $HOME/x-tools/aarch64-unknown-linux-gnu/aarch64-unknown-linux-gnu/sysroot/usr/include
make
sudo make install
```