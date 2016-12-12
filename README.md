# theShell
Desktop Environment written using the Qt toolkit

**Yay! theShell is now out of beta! :D**

## Screenshots
![Screenshot 1](https://raw.githubusercontent.com/vicr123/theshell/master/images/desktop.png)

## Dependencies
- qmake
- A EWMH compatible window manager. (KWin is the default)
- wmctrl
- xbacklight
- kscreen
- NetworkManager
- libsystemd
- akonadi (on Ubuntu, you'll need libkf5akonadi-dev - thanks SparkyCola)
- libcups
- libxcb
- libxcursor
- [tsscreenlock](https://github.com/vicr123/tsscreenlock)
- [ts-polkitagent](https://github.com/vicr123/ts-polkitagent)
- [ts-bt](https://github.com/vicr123/ts-bt)
- [ts-qtplatform](https://github.com/vicr123/ts-qtplatform)
- alsa-utils (optional, for volume controls)
- pocketsphinx (optional, for voice control)
- festival (optional, for voice control)

## Build
```
qmake
make
```

## Install
1. Copy theshell and init_theshell over to your binaries folder (usually /usr/bin)
2. Copy theshell.desktop to your xsessions folder (usually /usr/share/xsessions)

## Starting
- Use init_theshell in an X session to start theShell
- Use a display manager and use "theShell" as the session

## Packages
theShell is available in Arch Linux on the AUR under the name "[theshell](https://aur.archlinux.org/packages/theshell/)." This also pulls in all the required dependencies for theShell so it should work properly.

## Bugs/Feature Requests
- Report any bugs using the "Issues" tab up there. Alternatively, click [here](https://github.com/vicr123/theshell/issues) to jump straight there. (Thanks!)
- If you want to request a feature, you can also use the "Issues" tab.

## Blueprint
- theShell has a super unstable untested version called "blueprint." Check out the [blueprint](https://github.com/vicr123/theshell/tree/blueprint) branch for more information.
  - The Blueprint branch has been untested. This is only recommended for early adopters and systems that aren't mission-critical. Don't use theShell Blueprint as your daily driver!
  - theShell and theShell Blueprint can be installed together. Just rename the binary and initialization script (a good name is theshell-b and init-theshell-b,) put them in your binaries folder, rename the .desktop file, change it to start the new initialization script and put it in your xsessions folder.

## Warnings
- theShell is only tested on Arch Linux. Your milage may vary on other distributions.

**Thanks for using theShell :D**
