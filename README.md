# ⚠️ It's the end of the road for theShell.
Now that theDesk has been released, it's time to say goodbye to theShell. [View Blog Post](https://vicr123.com/blog/2023/07/17/sunsetting-theshell)

If you're after a desktop that looks and works like theShell, please check out [theDesk](https://github.com/theCheeseboard/thedesk)!

---
# theShell
Desktop Environment written using the Qt toolkit

## Screenshots
![Screenshot 1](https://raw.githubusercontent.com/vicr123/theshell/master/shell/images/desktop.png)

## Dependencies
- qmake
- A EWMH compatible window manager. (KWin is the default)
- [the-libs](https://github.com/vicr123/the-libs)
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

## Build
```
qmake
make
```

## Install
```
make install
```

## Starting
- Use ts-startsession in an X session to start theShell
- Use a display manager and use "theShell" as the session

## Packages
theShell is available in Arch Linux in a custom package repository. Simply add the following lines to the bottom of `/etc/pacman.conf`
```
[theapps]
SigLevel = Optional TrustAll
Server = https://vicr123.github.io/repo/arch/$arch/
```
To install theShell, perform system updates with `pacman -Syu` first, and then `pacman -S theshell`.
theShell is also on the AUR under the name "[theshell](https://aur.archlinux.org/packages/theshell/)." This also pulls in all the required dependencies for theShell so it should work properly.

## Bugs/Feature Requests
- Report any bugs using the "Issues" tab up there. Alternatively, click [here](https://github.com/vicr123/theshell/issues) to jump straight there. (Thanks!)
- If you want to request a feature, you can also use the "Issues" tab.

## Translators
See ```TRANSLATORS.md``` for all the translators that helped make theShell available for all.

## Blueprint
- theShell has a super unstable untested version called "blueprint." Check out the [blueprint](https://github.com/vicr123/theshell/tree/blueprint) branch for more information.
  - The Blueprint branch has been untested. This is only recommended for early adopters and systems that aren't mission-critical. Don't use theShell Blueprint as your daily driver!
  - theShell and theShell Blueprint can be installed together. Just rename the binary and initialization script (a good name is theshell-b and init-theshell-b,) put them in your binaries folder, rename the .desktop file, change it to start the new initialization script and put it in your xsessions folder.

## Warnings
- theShell is only tested on Arch Linux. Your milage may vary on other distributions.

**Thanks for using theShell :D**
