---
title: Creating a Live USB
tsVersion: OS 6.1 Apr.
---

To try or install theShell OS, a live USB may be a viable option. Running theShell OS off a USB flash drive is much faster than running it off optical media, like a DVD.

# Preperation

Before you create a live USB, you'll need to check that your computer is able to boot off USB drives. Most modern computers and tablets can do this with no problem.

You may need to jump into your BIOS to change the boot order so that your device boots off a USB flash drive.

# Placing theShell OS on a USB drive

To start the process of creating a live USB for theShell OS, you'll need to download a disc image of theShell OS from the download page. You'll also need to make sure that the USB flash drive is large enough to hold the disc image that you downloaded. **This drive will be erased, so make sure no important information is on there.**

Connect your USB flash drive to your computer, and then follow the steps below.

## GUI (For Windows, MacOS/Mac OS X, and Linux)

Download and Install the latest version of Etcher from https://etcher.io/.

After its done, drag the .iso file into the Select Image box or click Select Image, and using the file picker, choose the .iso file you downloaded.

Now, insert your USB if you haven't already and make sure it is selected.
(Note: On Windows if you used that USB as a LiveUSB before it might ask you to format it. Say No as Etcher does this for you)

Finally click Flash!

After it is finished, you should be able to restart and boot from your LiveUSB!
## macOS / Mac OS X (Terminal)

Open the Terminal app, and type the following command to list all your disks. Take note of the one that looks like your flash drive by comparing the reported size.

```
diskutil list
```

The disk name should take the form of /dev/disk[x]. Next, unmount the drive. This makes sure that macOS isn't reading or writing from the drive while we're putting theShell OS on it.

```
diskutil unmount /dev/disk[x]
```

Now we're ready to actually start putting theShell OS onto your flash drive. Start by typing in the following command, but don't press [ENTER] just yet.

```
sudo dd if=
```

Then open the Finder and drag the disc image you downloaded to the terminal. Finish the command by typing in the following, and then pressing [ENTER]. Make sure there is a space behind "of"

```
 of=/dev/disk[x] bs=1m
```

**WARNING**: This command will erase your USB drive and put theShell OS on it. Any data currently on the drive will be irrecoverably deleted.

Once you're done with that, you should be able to remove your USB drive and boot theShell OS from it.

## Linux (Terminal)

Open a root terminal, and type the following command to list all your disks. Take note of the one that looks like your flash drive by comparing the reported size.

```
lsblk
```

The disk name should take the form of /dev/sd[x]. Next, unmount the drive. This makes sure that your system isn't reading or writing from the drive while we're putting theShell OS on it.

```
umount /dev/sd[x]
```

Now we're ready to actually start putting theShell OS onto your flash drive. Type in the following command to write theShell OS onto the USB drive.

```
dd if=/path/to/tsos.iso of=/dev/sd[x] bs=4M status=progress
```

**WARNING**: This command will erase your USB drive and put theShell OS on it. Any data currently on the drive will be irrecoverably deleted.

Once you're done with that, you should be able to remove your USB drive and boot theShell OS from it.
