# FastRC
A simple to configure and fast init system.

## What is FastRC?
FastRC is a Linux init system like systemd, runit, OpenRC etc. It is the first thing that Linux runs after it switches to userspace.

## Why FastRC?
FastRC is blazing-fast and a piece of cake to setup in your linux distro. FastRC is also super lightweight.

## How to compile FastRC?
### 1. Installing Tools
#### For Debian/Ubuntu-based systems:
```bash
sudo apt install gcc make libncurses-dev binutils
```
#### For Arch-based systems:
```bash
sudo pacman -S gcc make ncurses binutils
```
### 2. Configuring
#### After downloading the source code, to configure it:
```bash
make menuconfig
```
This will bring up a TUI for you to customize your init.
### 3. Compiling
#### After configuring, to compile it:
```bash
make
```
## How to implement FastRC?
### 1. Copying
#### Copy everything inside output/ into your initrd's /sbin folder, example:
```bash
cp output/* ../your-linux-initrd/sbin
```
### 2. Configuring
#### While inside your initrd's /etc folder, make a folder for FastRC:
```bash
mkdir fastrc
```
#### Now create an inittab (/etc/fastrc/inittab) like this:
```fastrc
# Example inittab for FastRC
# Main script/program
sysinit=/etc/fastrc/rcS
# Script/program that runs before poweroff
shutdown=/etc/fastrc/poweroff
# Script/program that runs before reboot
reboot=/etc/fastrc/poweroff
# Script/program that runs for ctrl+alt+del
ctrlaltdel=/sbin/reboot
# Script/program that runs forever
respawn=/bin/getty tty0
```
#### Then, create a configuration file (/etc/fastrc/conf) like this:
```fastrc
# Example configuration for FastRC
# Verbose messages (1 to enable, 0 to disable)
verbose=0
```

#### While inside your initrd's root (/) just link FastRC to /init
```bash
ln -s sbin/fastrc init
```
### Congrats, you have put FastRC to your linux distro! It should look like this:
```
initrd
├── etc/
│   └── fastrc/
│       ├── inittab
│       └── conf
├── sbin/
│   ├── fastrc
│   ├── fastctl
│   ├── poweroff -> symlink to fastctl
├── └── reboot -> symlink to fastctl
└── init -> symlink to sbin/fastrc
```
