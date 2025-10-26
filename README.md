# Apollo Save Tool (PS3)

[![Downloads][img_downloads]][app_downloads] [![Release][img_latest]][app_latest] [![License][img_license]][app_license]
[![Build package](https://github.com/bucanero/apollo-ps3/actions/workflows/build.yml/badge.svg)](https://github.com/bucanero/apollo-ps3/actions/workflows/build.yml)
[![Twitter](https://img.shields.io/twitter/follow/dparrino?label=Follow)](https://twitter.com/dparrino)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/bucanero/apollo-ps3)

**Apollo Save Tool** is an application to manage save-game files, trophies, and licenses on the PlayStation 3.

This homebrew application allows you to download, unlock, patch and resign save-game files directly on your PS3. It can also unlock and resign trophies, backup/restore licenses, and create PS2 Classic images.

![image](./docs/screenshots/screenshot_main.png)

**Comments, ideas, suggestions?** You can [contact me](https://github.com/bucanero/) on [Twitter](https://twitter.com/dparrino) and on [my website](http://www.bucanero.com.ar/).

# Features

* **Easy to use:** no crypto-key configuration or advanced setup needed.
* **Standalone:** no computer required, everything happens on the PS3.
* **Automatic settings:** auto-detection of PSID, IDPS and Account-ID settings.
* **Multi-user:** supports multiple user accounts.
* **Localization support:** French, Italian, Japanese, Portuguese, Spanish.

## Save Management

* **Save files listing:** quick access to all the save files on USB and the internal PS3 HDD (+ file details)
* **Save SFO unlocking:** allows the user to remove the copy-lock flag, enabling transfer of `Copying Prohibited` save files.
* **Save files patching:** supports [Bruteforce Save Data](https://bruteforcesavedata.forumms.net/) and Game Genie cheat patches to enhance your savegames.
* **Save PFD resigning:** allows the user to resign save files made by other users and consoles.
* **Save downloading:** easy access to an Online Database of save-game files to download straight to your PlayStation.
* **Save file conversion:** converts and resigns PS1 and PS2 save-game formats to PS3 `.PSV` format.

## Trophy Management

* **Trophy Set resigning:** resign trophy sets for the current user and console.
* **Trophy Set exporting:** easy backup of trophy sets to `.zip` and raw formats.
* **Trophy Set importing:** restore trophy sets from raw USB backups.
* **Trophy unlocking:** allows to unlock individual trophies for the current user.

## Backup Management

* **Account activation:** create fake Account ID and generate offline PS3 activation (`act.dat`)
* **License exporting:** easy backup of user licenses to `.zip` and `.rap` formats.
* **License importing:** allows to import `.rap` files and generate user `.rif` licenses.

## PS2 Classics Management

* **Memory card exporting:** decrypt and export `.VME` memory cards.
* **Memory card importing:** allows to import `.VM2` files to encrypted `.VME` memcards.
* **ISO importing:** import PS2 `.ISO` files to `.BIN.ENC` encrypted image format.
* **CONFIG importing:** import PS2 `.CONFIG` files to `.ENC` encrypted format.
* **BIN.ENC exporting:** decrypt and export `BIN.ENC` images to `.ISO`.

## PS1 Virtual Memory Card Management

* **VMC saves management:** quick access to all save files on Virtual Memory Cards images.
  - Supported PS1 VMC formats: `.VMP`, `.MCR`, `.VM1`, `.BIN`, `.VMC`, `.GME`, `.VGS`, `.SRM`, `.MCD`
* **Import PS1 saves:** import saves to PS1 VMCs from other systems and consoles (`.MCS`, `.PSV`, `.PSX`, `.PS1`, `.MCB`, `.PDA` supported).
* **Export PS1 saves:** allows the user export saves on VMC images to `.MCS`/`.PSV`/`.PSX` formats.

## PS2 Virtual Memory Card Management

* **VMC saves management:** quick access to all save files on Virtual Memory Cards images.
  - Supported PS2 VMC formats: `.VM2`, `.VME`, `.BIN`, `.VMC`
  - Supports ECC and non-ECC images, and PS2 Classics encryption
* **Import PS2 saves:** import saves to PS2 VMCs from other systems and consoles (`.PSU`, `.PSV`, `.XPS`, `.CBS`, `.MAX`, `.SPS` supported).
* **Export PS2 saves:** allows the user export saves on VMC images to `.PSU` and `.PSV` formats.

# Download

Get the [latest version here][app_latest].

## Changelog

See the [latest changes here](CHANGELOG.md).

# Donations

My GitHub projects are open to a [sponsor program](https://patreon.com/dparrino). If you feel that my tools helped you in some way or you would like to support it, you can consider a [PayPal donation](https://www.paypal.me/bucanerodev).

# Setup instructions

No special setup is needed. Just download the latest `apollo-ps3.pkg` package and install it on your PlayStation 3.
On first run, the application will detect and setup the required user settings.

## Data folders

### PS3

| PS3 | Folder |
|-----|--------|
| **USB saves** | your files must be placed on `/dev_usb00x/PS3/SAVEDATA/`. |
| **HDD saves** | files will be scanned from `/dev_hdd0/home/000000XX/savedata/`, where `XX` is the current `User ID`. |
| **HDD licenses** | `/dev_hdd0/home/000000XX/exdata/` (`*.rif`) |
| **USB licenses** | `/dev_usb00x/exdata/` (`*.rap`) |

### PS2

| PS2 | Folder |
|-----|--------|
| **USB saves** | `/dev_usb00x/PS2/SAVEDATA/` (`*.xps`, `*.max`, `*.psu`, `*.cbs`, `*.sps`) |
| **VMC cards** | `/dev_usb00x/PS2/VMC/` (`*.vmc`, `*.vme`, `*.vm2`, `*.bin`) |
| **VME cards** | `/dev_usb00x/PS3/EXPORT/PS2SD/` |
| **PSV saves** | `/dev_usb00x/PS3/EXPORT/PSV/` |
| **HDD VME cards** | `/dev_hdd0/home/000000XX/ps2emu2_savedata/`, where `XX` is the current `User ID`. |
| **HDD VM2 cards** | `/dev_hdd0/savedata/vmc/` |
| **HDD ISOs** | `/dev_hdd0/PS2ISO/` (`*.bin`, `*.iso`, `*.bin.enc`) |
| **USB ISOs** | `/dev_usb00x/PS2ISO/` (`*.bin`, `*.iso`, `*.bin.enc`) |

### PS1

| PS1 | Folder |
|-----|--------|
| **USB saves** | `/dev_usb00x/PS1/SAVEDATA/` (`*.mcs`, `*.psx`) |
| **PSV saves** | `/dev_usb00x/PS3/EXPORT/PSV/` |
| **HDD VM1 cards** | `/dev_hdd0/savedata/vmc/` |
| **USB VMC cards** | `/dev_usb00x/PS1/VMC/` (`*.mcr`, `*.vmp`, `*.bin`, `*.vmc`, `*.gme`, `*.vgs`, `*.srm`, `*.mcd`) |

# Usage

Using the application is simple and straight-forward: 

 - Move <kbd>UP</kbd>/<kbd>DOWN</kbd> to select the save-game file you want to patch, and press ![X button](https://github.com/bucanero/pkgi-ps3/raw/master/data/CROSS.png). The patch screen will show the available fixes for the file. Select the patches and click `Apply`.
 - To view the item's details, press ![Triangle](https://github.com/bucanero/pkgi-ps3/raw/master/data/TRIANGLE.png).
It will open the context menu on the screen. Press ![O button](https://github.com/bucanero/pkgi-ps3/raw/master/data/CIRCLE.png) to return to the list.
 - To reload the list, press ![Square](https://github.com/bucanero/pkgi-ps3/raw/master/data/SQUARE.png).
 - Press <kbd>L1</kbd>/<kbd>L2</kbd> or <kbd>R1</kbd>/<kbd>R2</kbd> trigger buttons to move pages up or down.

## Overriding auto-detected settings

If you want to override the auto-detected IDs used by Apollo to resign the save-games, you can use the `owners.xml` file.
For example:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<apollo version="1.2.0">
  <owner name="PS3 User">
    <console idps="AAAAAAAAAAAAAAAA 0123456789ABCDEF" psid="FFFFFFFFFFFFFFFF 0123456789ABCDEF"/>
    <user id="00000002" account_id="0123456789abcdef"/>
  </owner>
  <owner name="Other User">
    <console idps="AAAAAAAAAAAAAAAA 0123456789ABCDEF" psid="FFFFFFFFFFFFFFFF 0123456789ABCDEF"/>
    <user id="00000008" account_id="abcdef0123456789"/>
  </owner>
</apollo>
```

The file must be saved on `/dev_hdd0/game/NP0APOLLO/USRDIR/owners.xml`. Apollo will load the hard-coded values when loading a save, and you can 
select the desired account from the **Change Account ID** menu under the `SFO Patches` options.

In the `owners.xml` you need to define:
- Owner name
- User ID (e.g.: `00000123`)
- Account ID (e.g.: `0123456789abcdef`)
- Console PSID (e.g.: `FFFFFFFFFFFFFFFF 0123456789ABCDEF`)
- Console IDPS (required for license import/export)

# Online Database

The application also provides direct access to the [Apollo online database](https://github.com/bucanero/apollo-saves) of save-game files for PlayStation 3 games. These usually offer additional features such as completed games that can save you many hours of playing.

Currently, the list of available games and files is limited, but the project aims to add more save-games shared by the community.

**Note:** Downloaded save files **must be resigned** using Apollo before loading them in your games.

# FAQs

 1. Where I can get a save-game for *XYZ game*?
    
    You can check sites like [Brewology.com](https://ps3.brewology.com/gamesaves/savedgames.php?page=savedgames&system=ps3), and [GameFAQs](https://gamefaqs.gamespot.com/ps3/). Also, searching on [Google](http://www.google.com) might help.
 1. I have a save-game file that I want to share. How can I upload it?
    
    If you have a save file that is not currently available on the Online Database and want to share it, please check [this link](https://github.com/bucanero/apollo-saves) for instructions.
 1. Why is it called **Apollo**?
    
    [Apollo](https://en.wikipedia.org/wiki/Apollo) was the twin brother of [Artemis](https://en.wikipedia.org/wiki/Artemis), goddess of the hunt. Since this project was born using the [Artemis-GUI](https://github.com/Dnawrkshp/ArtemisPS3/) codebase, I decided to respect that heritage by calling it Apollo.

# Credits

* [Bucanero](https://github.com/bucanero): [Project developer](https://bucanero.github.io/apollo-ps3/)

## Acknowledgments

* [Dnawrkshp](https://github.com/Dnawrkshp/): [Artemis PS3](https://github.com/Dnawrkshp/ArtemisPS3)
* [Berion](https://www.psx-place.com/members/berion.1431/): GUI design
* [flatz](https://github.com/flatz): [SFO/PFD tools](https://github.com/bucanero/pfd_sfo_tools/)
* [aldostools](https://aldostools.org/): [Bruteforce Save Data](https://bruteforcesavedata.forumms.net/)
* [darkautism](https://darkautism.blogspot.com/): [PS3TrophyIsGood](https://github.com/darkautism/PS3TrophyIsGood)
* [jimmikaelkael](https://github.com/jimmikaelkael): ps3mca tool
* [ShendoXT](https://github.com/ShendoXT): [MemcardRex](https://github.com/ShendoXT/memcardrex)
* [Nobody/Wild Light](https://github.com/nobodo): [S3M music track](https://github.com/bucanero/apollo-ps3/blob/master/data/haiku.s3m)

## Translators

Apollo supports multiple languages for its user interface. Thanks to the following contributors for their help with translations:

- Algol (French), [Bucanero](https://github.com/bucanero) (Spanish), [TheheroGAC](https://x.com/TheheroGAC) (Italian), [yyoossk](https://x.com/Cloud0835) (Japanese), [Phoenixx1202](https://github.com/Phoenixx1202) (Portuguese), SpyroMancer (Greek)

# Building

You need to have installed:

- [PS3 toolchain](https://github.com/bucanero/ps3toolchain)
- [PSL1GHT](https://github.com/ps3dev/PSL1GHT) SDK
- [Apollo](https://github.com/bucanero/apollo-lib) library
- [Tiny3D](https://github.com/wargio/Tiny3D) library
- [polarSSL](https://github.com/bucanero/ps3libraries/blob/master/scripts/015-polarssl-1.3.9.sh) library
- [libcurl](https://github.com/bucanero/ps3libraries/blob/master/scripts/016-libcurl-7.64.1.sh) library
- [libxmp-lite](https://github.com/bucanero/libxmp-lite-ps4) library
- [mini18n](https://github.com/bucanero/mini18n) library
- [dbglogger](https://github.com/bucanero/dbglogger) library

Run `make` to create a release build. If you want to include the latest save patches in your `.pkg` file, run `make createzip`.
Finally, run `make pkg` to create a `.pkg` install file.

You can also set the `PS3LOAD` environment variable to the PS3 IP address: `export PS3LOAD=tcp:x.x.x.x`.
This will allow you to use `make run` and send `apollo-ps3.self` directly to the [PS3Load listener](https://github.com/bucanero/ps3loadx).

To enable debug logging, build Apollo Save Tool with `make DEBUGLOG=1`. The application will send debug messages to
UDP multicast address `239.255.0.100:30000`. To receive them you can use [socat][] on your computer:

    $ socat udp4-recv:30000,ip-add-membership=239.255.0.100:0.0.0.0 -

# License

[Apollo Save Tool](https://github.com/bucanero/apollo-ps3/) (PS3) - Copyright (C) 2020-2025 [Damian Parrino](https://twitter.com/dparrino)

This program is free software: you can redistribute it and/or modify
it under the terms of the [GNU General Public License][app_license] as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

[socat]: http://www.dest-unreach.org/socat/
[app_downloads]: https://github.com/bucanero/apollo-ps3/releases
[app_latest]: https://github.com/bucanero/apollo-ps3/releases/latest
[app_license]: https://github.com/bucanero/apollo-ps3/blob/master/LICENSE
[img_downloads]: https://img.shields.io/github/downloads/bucanero/apollo-ps3/total.svg?maxAge=3600
[img_latest]: https://img.shields.io/github/release/bucanero/apollo-ps3.svg?maxAge=3600
[img_license]: https://img.shields.io/github/license/bucanero/apollo-ps3.svg?maxAge=2592000
