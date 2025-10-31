# Changelog

All notable changes to the `apollo-ps3` project will be documented in this file. This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]()

---

## [v2.2.4](https://github.com/bucanero/apollo-ps3/releases/tag/v2.2.4) - 2025-11-02

### Added

* Localization support
  - Auto-detect system language setting
  - Languages: English, French, Italian, Japanese, Portuguese, Russian, Spanish
* Show save details window for items in Online DB (`Triangle` button)
* Improved ARMax PS2 save-game conversion to `.PSV`

### Misc

* Updated Apollo Patch Engine to v1.4.0
  - Update custom mod tags
  - Support multiple mod options per line
  - Add AES CTR, Blowfish CBC encryption
  - Add DBZ Xenoverse 2 custom checksum

---

## [v2.2.0](https://github.com/bucanero/apollo-ps3/releases/tag/v2.2.0) - 2025-04-12

### Added

* FTP Server support (Saves Cloud Backup)
  - Zip, Upload, and backup saves to a user-defined FTP server
  - List, Download, and restore saves from a user-defined FTP server
  - Backup PS1, PS2, and PS3 saves
* Improved `Copy save to HDD` option
* Settings Menu:
  - Added `FTP Server URL` option
  - Added selector option (`FTP/Online DB`) for save-game downloads
  - Added `USB Source` selector option
  - Persistent `Debug log` option
  - Removed `Save Owner` option

### Misc

* Updated Apollo Patch Engine to v1.3.0
  - Add `djb2` hash function
  - Add tag support for user-selected options on `.savepatch` files
    - E.g., `{ZZ}val=name;03E7=999 gold;...;270F=9999 gold{/ZZ}`
    - Support for SaveWizard and BSD codes with multiple options

---

## [v2.0.2](https://github.com/bucanero/apollo-ps3/releases/tag/v2.0.2) - 2024-11-23

### Added

* Delete PS3 saves from USB
* PS2 Virtual Memory Card images (VMC)
  - Support additional import formats: `.XPS`, `.CBS`, `.MAX`, `.SPS`
* Online DB: show PS1 icons
* User Tools:
  - Convert PS1/PS2 saves to `.PSV` format
  - Convert `.PSV` saves to `.MCS`/`.PSU` formats
  - Re-sign `.PSV` saves

### Fixed

* "Copy to HDD" option now only available when browsing USB saves

### Misc

* Updated Apollo Patch Engine to v1.1.2
  - Fixed SW Code search bug when bytes are not found
* Option to launch an external app
  - (L1 + START) executes `/dev_hdd0/game/NP0APOLLO/USRDIR/EXTRA.SELF`

---

## [v2.0.0](https://github.com/bucanero/apollo-ps3/releases/tag/v2.0.0) - 2024-04-10

### Added

* Manage PS1 Virtual Memory Card images (VMC)
  - Supports `.VM1` and external formats (`.MCR`, `.VMP`, `.BIN`, `.VMC`, `.GME`, `.VGS`, `.SRM`, `.MCD`)
  - List, import, export, and delete PS1 saves inside VMC images
  - Import - Supported formats: `.MCS`, `.PSV`, `.PSX`, `.PS1`, `.MCB`, `.PDA`
  - Export - Supported formats: `.MCS`, `.PSV`, `.PSX`
* Manage PS2 Virtual Memory Card images (VMC)
  - Supports `.VM2`, ECC and non-ECC images (`.PS2`, `.VME`, `.BIN`, `.VMC`)
  - List, import, export, and delete PS2 saves inside VMC images
  - Import - Supported formats: `.PSU`, `.PSV`
  - Export - Supported formats: `.PSU`, `.PSV`
  - Handle PS2 Classics encryption on VME images
* Save sort option by Type (PS3/PS1/PS2/PSP/VMC)
* Online DB: added PS1 saves listing
* Custom save decryption support
  - Metal Gear Solid 5: Ground Zeroes
* New Secure file keys
  - FIFA 18
  - FIFA 19

### Fixed

* Patch Code fixes
  - Metal Gear Solid 5: The Phantom Pain
  - Resident Evil 6

### Misc

* Updated Apollo Patch Engine to v1.0.0
  - Add `jenkins_oaat`, `lookup3_little2` hash functions
  - Add `camellia_ecb` encryption
  - Add RGG Studio decryption (PS4)
  - Add Dead Rising checksum
  - Fix SaveWizard Code Type D issue with `CRLF` line breaks
  - Changed BSD command syntax for `decompress(offset, wbits)` and `compress(offset)`

---

## [v1.8.7](https://github.com/bucanero/apollo-ps3/releases/tag/v1.8.7) - 2023-12-03

### Added

* New cheat codes
  - Assassin's Creed II

### Fixed

* Fix PS2 Classics encryption/decryption progress bar

### Misc

* Update Web Server saves index page

---

## [v1.8.6](https://github.com/bucanero/apollo-ps3/releases/tag/v1.8.6) - 2023-11-25

### Added

* Auto-detect `X`/`O` button settings
* "Copy save to HDD" supports overwriting
* Allow fake account re-activation
* New cheat codes
  - Assassin's Creed II
  - Grand Theft Auto V
  - Metal Gear Solid 5: The Phantom Pain
* Custom decryption support
  - Metal Gear Solid 5: The Phantom Pain
* Custom checksum support
  - Assassin's Creed
  - Assassin's Creed II
  - Assassin's Creed III
  - Assassin's Creed IV: Black Flag
  - Assassin's Creed: Brotherhood
  - Assassin's Creed: Revelations
  - Assassin's Creed Rogue
  - Castlevania: Lords Of Shadow
  - Castlevania: Lords Of Shadow 2
  - Grand Theft Auto V
  - Metal Gear Solid 5: The Phantom Pain

### Fixed

* Fix exported file attributes on FAT32 drives
* Fix `BCAS20224` secure file-id key

### Misc

* Network HTTP proxy settings support
* Updated [`apollo-lib`](https://github.com/bucanero/apollo-lib) Patch Engine to v0.6.0
  - Add host callbacks (username, system name, wlan mac, psid, account id)
  - Add `murmu3_32`, `jhash` hash functions
  - Add Patapon 3 PSP decryption
  - Add MGS5 decryption (PS3/PS4)
  - Add Monster Hunter 2G/3rd PSP decryption
  - Add Castlevania:LoS checksum
  - Add Rockstar checksum
  - Fix SaveWizard Code Type C
  - Fix `right()` on little-endian platforms

---

## [v1.8.4](https://github.com/bucanero/apollo-ps3/releases/tag/v1.8.4) - 2023-04-08

### Added

* Hex Editor for save-data files
* Improved internal Web Server (Online DB support)
* Custom offline Account-ID activation (On-screen Keyboard)
* User-defined Online DB URL (`Settings`)

### Fixed

* Fix Final Fantasy XIII save-game decryption bug

### Misc

* Updated Apollo patch engine v0.4.1
  * Skip search if pattern was not found
  * Improve code types 9, B, D
  * Fixed Final Fantasy XIII encryption bug on PS3
  * Add value subtraction support (BSD)

---

## [v1.8.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.8.0) - 2023-01-28

### Added

* Import Trophies from USB
* Improve Trophy bulk export
* Download Online DB saves to HDD
* Extract archive tool (`.zip`, `.7z`, `.rar`)
* Network Tools
  * URL downloader tool (download http/https/ftp/ftps links)
  * Simple local Web Server (full access to console drives)
* New save-game sorting options (`Settings`)
  * by Name, by Title ID
* Show PS3 IP address when running Apollo's Web Server
* Improve USB support (scan USB0 to USB6)
* On-screen Keyboard (for text input)

### Fixed

* Fix screen aspect ratio (Tiny3D viewport issue)
* Fix list refresh bug with `.PSV` saves
* Fix `.VM2` to `.VMC` export bug
* Fix bug when saving settings

### Misc

* Download application data updates from `apollo-patches` repository
* Use `.savepatch` extension for save-game patches
* Updated Apollo patch engine v0.3.0
  * BSD scripting changes
  * New custom hash: `mgspw_checksum`
  * Fix `sha1_xor64` custom hashing
  * Fix little-endian support for decrypters/hashes (PS4/Vita)

---

## [v1.7.4](https://github.com/bucanero/apollo-ps3/releases/tag/v1.7.4) - 2022-09-28

### Added

* New cheat codes
  * Saint Seiya Sanctuary Battle
* New custom checksum support
  * Dragon Age: Origins
  * Dynasty Warriors: Gundam
  * LEGO Harry Potter: Years 1-4
  * LEGO Indiana Jones 2: The Adventure Continues
  * The Orange Box (Half-Life, Portal, TF2)

### Misc

* Updated Apollo patch engine v0.2.0
  * Improve patch error handling
  * New commands: `copy`, `endian_swap`, `msgbox`
  * New custom hash: `force_crc32`
  * Improve SW code types 9, A
  * Add SW code types 3, 7, B, C, D
  * Support initial value for `add/wadd/dwadd/wsub`
  * Fix `md5_xor` custom hash
* Updated network code to `libcurl` (TLS 1.2) 
* Changed background music using `libxmp`

### Fixed

* Improved Account/Console ID re-detection

---

## [v1.7.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.7.0) - 2022-04-28

### Added

* Added local Web Server (Download saves as .Zip)
* Save-game selection (press `SELECT` to tag items)
  * Copy/Resign selected saves (Bulk Management)
* Improved listings (items count)
* New custom checksum support
  * LEGO Marvel Avengers
  * LEGO Star Wars: The Force Awakens

---

## [v1.6.5](https://github.com/bucanero/apollo-ps3/releases/tag/v1.6.5) - 2022-01-22

### Added

* New save-game cheats
  * Final Fantasy XIII
  * Ratchet & Clank Future: A Crack in Time
  * Resident Evil 4 HD
  * Resident Evil 6
  * Resident Evil Revelations 2
  * Ryu ga Gotoku Kiwami
  * Tales of Zestiria

### Fixed

* Fixed Online DB URL

### Misc

* Use Apollo-lib (shared with Apollo PS4)

---

## [v1.6.4](https://github.com/bucanero/apollo-ps3/releases/tag/v1.6.4) - 2021-07-19

### Added

* Added setting to redetect user/console IDs
* Added custom decryption support
  * Dynasty Warriors 8: Xtreme Legends
* Added custom checksum support
  * LEGO Pirates of the Caribbean: The Video Game
  * LEGO Star Wars III: The Clone Wars
  * Tales of Zestiria
* New save-game cheats
  * Atelier Rorona Plus
  * Atelier Shallie
  * Battle of Tiles EX
  * Call of Duty: World at War
  * Class of Heroes 2G
  * Death Spank
  * Deathmatch Village
  * DuckTales Remastered
  * Everybody's Golf
  * Iron Man 2
  * J-STARS Victory VS+
  * Kick Beat
  * Monster High - New Ghoul in School
  * Mortal Kombat Arcade Kollection
  * Narco Terror
  * Sacred Citadel
  * Shrek Forever After
  * Silent Hill: Downpour
  * Super Motherload
  * Tales of Zestiria
  * The Awakened Fate Ultimatum
  * The Expendables 2
  * Vandal Hearts: Flames of Judgment
  * Wizardry The Labyrinth Of Lost Souls

### Fixed

* Fixed freeze issue when db rebuild was scheduled
* Fixed comments in `ps3savepatch` files

---

## [v1.6.3](https://github.com/bucanero/apollo-ps3/releases/tag/v1.6.3) - 2021-07-06

### Added

* Added offline PS3 account activation
  * Creates a fake account ID
  * Generates an offline activation file `act.dat`
* View raw `.ps3savepatch` patch files
* Added BSD `mid`, `endian_swap` commands support
* Added `fnv1`, `checksum32`, `wadd_le`, `sha1_xor64` hash methods
* Added custom decryption support
  * Metal Gear Solid 2
  * Metal Gear Solid 3
  * Metal Gear Solid Peace Walker
* Added custom checksum support
  * Alien: Isolation
  * Digimon All-Star Rumble
  * Dragon Ball Z: Ultimate Tenkaichi
  * DuckTales: Remastered
  * LEGO Batman 2: DC Super Heroes
  * LEGO Harry Potter: Years 1-4
  * LEGO Harry Potter: Years 5-7
  * LEGO Lord Of The Rings
  * LEGO The Hobbit
  * Samurai Warriors 4
  * Sengoku Musou 4
  * Tears to Tiara 2
* New save-game cheats
  * Dragon Ball Z: Xenoverse
  * Odin Sphere Leifthrasir
  * One Piece Pirate Warriors 3
  * Sengoku Basara 4 Sumeragi
  * Transformers: Devastation

---

## [v1.6.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.6.0) - 2021-05-22

### Added

* Display save-games and trophy-set icons
* Added Database rebuild dialog (trophies)
* Added PS2 Classics `CONFIG` encryption support
* Improved USB scanning for "User Backup" tools

### Fixed

* Fixed `.ps3savepatch` file parsing

---

## [v1.5.8](https://github.com/bucanero/apollo-ps3/releases/tag/v1.5.8) - 2021-04-03

### Added

* Added Trophy edit support
  * Unlock trophies
  * Lock trophies (if not synced)
* Added bulk trophy export to `.Zip`
* Added custom checksum support
  * Kingdom Hearts Chain of Memories
* Improved file copy to USB

### Fixed

* Fixed `account_id` update (special case)

---

## [v1.5.5](https://github.com/bucanero/apollo-ps3/releases/tag/v1.5.5) - 2021-03-17

### Added

* Added BSD `qwadd`, `right`, `left` commands support
* Added `adler16`, `sdbm`, `dwadd_le` hash methods
* Added `dev_usb006` scan for USB saves
* Added custom decryption support
  * Final Fantasy XIII
  * Final Fantasy XIII-2
  * Lightning Returns: Final Fantasy XIII
  * Resident Evil 0 (Zero) (Blowfish)
  * Resident Evil HD Remaster (Blowfish)
  * Resident Evil Revelations 2 (Blowfish)
  * Need for Speed: Undercover
  * Call of Duty: Black Ops (3-DES CBC)
  * Call of Duty: Black Ops II (3-DES CBC)
  * Silent Hill 3 HD
* New save-game cheats
  * Persona 5
  * Resident Evil: Revelations 2
  * Yakuza 0
  * 99 Vidas
  * Lost Dimension
  * RAGE
  * The Last of Us
  * BlazBlue
  * BlazBlue: Chronophantasma EXTEND
  * BlazBlue: Central Fiction
  * Resident Evil 0 (Zero)
  * Resident Evil 4 HD
  * Resident Evil 5
  * Resident Evil 6
  * The Elder Scrolls V: Skyrim
  * The Legend of Heroes: Sen no Kiseki
  * Devil May Cry 3 HD
  * Guilty Gear Xrd - Rev. 2 -
  * Dynasty Warriors 8: Empires
  * Saint Seiya Soldiers' Soul
  * Star Ocean 5: Integrity and Faithlessness
  * Kamen Rider Battride War Sousei/Genesis
  * Rainbow Skies
* Added custom checksum support
  * Dead Space 2
  * Dead Space 3
  * Final Fantasy X HD
  * Final Fantasy X-2 HD
  * Final Fantasy XIII
  * Final Fantasy XIII-2
  * Lightning Returns: Final Fantasy XIII
  * Kingdom Hearts 2.5 HD
  * Kingdom Hearts Birth by Sleep
  * Prototype
  * Prototype 2
* Added simple icons for trophies
* Improved custom CRC32 method

### Fixed

* Fixed bulk resign and bulk copy
* Fixed Fake owner account (`ffffffffffffffff`)

---

## [v1.5.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.5.0) - 2021-01-31

### Added

* Added Trophy Set management
  * List trophies
  * Resign trophy sets
  * Backup trophies to Zip
* Added custom encryption support (Blowfish, AES, DES)
  * Diablo 3
  * DmC: Devil May Cry (Blowfish)
  * Grand Theft Auto V (AES CBC)
  * Need for Speed Rivals (Blowfish)
* Added "Import decrypted save file" option
* Added PS2/PS1 `.PSV` saves to the Online Database
* Improved user messages
* Added BSD global variable support
* Added BSD `search next` command support
* Added GameGenie code type `88` relative search 

### Fixed

* Fixed `.RAP` uppercase issue

---

## [v1.4.6](https://github.com/bucanero/apollo-ps3/releases/tag/v1.4.6) - 2020-12-23

### Added

* Added BSD `crc64_ecma` and `crc64_iso` hash commands
* Added custom CRC-64 hash support (see `BLES01724.ps3savepatch` example)
* Added `deflate` support to BSD `decompression` command

### Fixed

* Fixed save-game patches
  * Darksiders
  * Call of Juarez Bound in Blood
  * Fuse

---

## [v1.4.5](https://github.com/bucanero/apollo-ps3/releases/tag/v1.4.5) - 2020-12-13

### Added

* Added `Change Account ID` option (Remove ID, Fake Owner/Rebug)
* Added background to PS3 system Dialogs
* Added `Enable Debug Log` setting
* Game Genie Type 6 code support
* New save-game cheats
  * Guilty Gear Xrd -SIGN-
  * Guilty Gear Xrd Rev 2
  * BlazBlue: Central fiction
* Updated `secure_file_id` database with new Title IDs and keys
* Updated Online Database with new save-games

### Fixed

* Fixed Online Database save-game download bug

---

## [v1.4.2](https://github.com/bucanero/apollo-ps3/releases/tag/v1.4.2) - 2020-09-18

### Added

* Export PS1 `.PSV` saves to `.mcs`
* Export PS2 `.PSV` saves to `.psu`
* Import raw PS2 memcards to `.VM2` (ECC / non-ECC)
* Export PS2 `.VM2` memcards to raw image
* Added `APOLLO-99PS1.PSV` dummy save (to refresh XMB virtual memcards)

---

## [v1.4.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.4.0) - 2020-09-12

### Added

* PS2 Classics: export/decrypt `.VME` memcards  to `.VM2`
* PS2 Classics: import/encrypt `.VM2` memcards to `.VME`
* PS2 Classics: export/decrypt `BIN.ENC` images to `.ISO`
* PS2 Classics: import/encrypt `.ISO` images to `BIN.ENC`
* Added `.PSV` resigning (PS1/PS2)
* Added PS1 `.mcs`/`.psx` save conversion to `.PSV`
* Added PS2 `.max`/`.cbs`/`.xps`/`.psu`/`.sps` save conversion to `.PSV`
* Added `APOLLO-99PS2.PSV` dummy save (to refresh XMB virtual memcards)

---

## [v1.2.5](https://github.com/bucanero/apollo-ps3/releases/tag/v1.2.5) - 2020-07-26

### Added

* Updated `secure_file_id` database with new IDs and keys
* Add `Copy savegame to HDD` option
* Add `Zip savegame to HDD` option
* Add bulk save-game copy to HDD support
* Add `owners.xml` support (select owners from the Settings menu)

---

## [v1.2.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.2.0) - 2020-07-18

### Added

* Add save-game region change support
* Add custom decryption for Naughty Dog savegames
* Add bulk save-game resign support
* Add View Save details option
* Add Export licenses to HDD
* Add proper detection of unprotected games
* Add analog stick control support

### Fixed

* Removed unsupported cheat code files
* Fix BSD cheat code applying process
* No cheat codes selected by default

---

## [v1.1.2](https://github.com/bucanero/apollo-ps3/releases/tag/v1.1.2) - 2020-07-04

### Added

* Export single `.rif` license to `.rap` file (User Backup menu)
* Import single `.rap` file to user's `.rif` license (User Backup menu)
* Bulk import `.rap` files to user's content `.rif` licenses (User Backup menu)

---

## [v1.1.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.1.0) - 2020-06-14

### Added

* Exporting user's content licenses to `.rap` files (User Backup menu)

---

## [v1.0.9](https://github.com/bucanero/apollo-ps3/releases/tag/v1.0.9) - 2020-04-26

### Added

* Support for compressed save-game file patching
* Added `insert`, `delete`, `compress`, and `decompress` BSD patch commands
* Improved save list browsing
* Improved BSD patch code parsing
* Reduced package installer size

---

## [v1.0.2](https://github.com/bucanero/apollo-ps3/releases/tag/v1.0.2) - 2020-03-25

### Added

* Re-added resign/cheat options to HDD/USB
* Added decrypt file option
* Updated UI with custom font

### Fixed

* Fixed game listing issue (when name has a `\n`)
* Fixed custom CRC calculation bug (BSD patches)

---

## [v1.0.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.0.0) - 2020-03-15

### Added

* [Bruteforce Save Data](https://bruteforcesavedata.forumms.net/) cheat patch support
* Changed save-game encryption/decryption method
* Splash screen logo

---

## [v0.9.1](https://github.com/bucanero/apollo-ps3/releases/tag/v0.9.1) - 2020-03-10

Hot fix release.

### Fixed

* Fixed bug when resigning a save-game on USB
* Fixed bug when selecting `View Details` on an empty save list

---

## [v0.9.0](https://github.com/bucanero/apollo-ps3/releases/tag/v0.9.0) - 2020-03-07

### Added

* Game Genie cheat patch support
* Updated UI
* Added `Remove Console ID` patch
* Added Licenses backup to .Zip (`/dev_hdd0/home/000000XX/exdata/`)
* Added bulk save-game copy to USB
* Added Trophies backup to USB
* Added `owner.txt` support to override auto-detected settings
* Improved auto-update check/download

### Fixed

* Solved issue when copying save-games from HDD to USB
* Solved freeze bug when using `Clear local cache`
* UI: Fixed improper titles and messages

---

## [v0.6.5](https://github.com/bucanero/apollo-ps3/releases/tag/v0.6.5) - 2020-02-17

### Added

* Export save-game to .Zip file
* Copy save-game to USB
* New `Settings` option to clear local cache
* New `Settings` option to update application data

### Fixed

* Fixed patch when removing Account ID
* Use internal PS3 fonts to fix issues with extended characters

---

## [v0.5.1](https://github.com/bucanero/apollo-ps3/releases/tag/v0.5.1) - 2020-02-09

Hot fix release.

### Fixed

* Fix unzip issue when unpacking online save files

---

## [v0.5.0](https://github.com/bucanero/apollo-ps3/releases/tag/v0.5.0) - 2020-02-07

First public release.

### Added

* Save file listing (+ details)
* Save file SFO unlocking (remove lock flag)
* Save file PFD resigning
* Save file download from Online Database
* Automatic detection of PSID/Account-ID settings
