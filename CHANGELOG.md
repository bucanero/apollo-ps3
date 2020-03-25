# Changelog

All notable changes to the `apollo-ps3` project will be documented in this file. This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]()

### Added

* re-added resign/cheat options to HDD/USB
* Added decrypt file option

### Fixed

* Fixed game listing issue (when name has a `\n`)
* Fixed custom CRC calculation bug (BSD patches)

## [v1.0.0](https://github.com/bucanero/apollo-ps3/releases/tag/v1.0.0) - 2020-03-15

### Added

* [Bruteforce Save Data](https://bruteforcesavedata.forumms.net/) cheat patch support
* Changed save-game encryption/decryption method
* Splash screen logo

## [v0.9.1](https://github.com/bucanero/apollo-ps3/releases/tag/v0.9.1) - 2020-03-10

Hot fix release.

### Fixed

* Fixed bug when resigning a save-game on USB
* Fixed bug when selecting `View Details` on an empty save list

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

## [v0.6.5](https://github.com/bucanero/apollo-ps3/releases/tag/v0.6.5) - 2020-02-17

### Added

* Export save-game to .Zip file
* Copy save-game to USB
* New `Settings` option to clear local cache
* New `Settings` option to update application data

### Fixed

* Fixed patch when removing Account ID
* Use internal PS3 fonts to fix issues with extended characters

## [v0.5.1](https://github.com/bucanero/apollo-ps3/releases/tag/v0.5.1) - 2020-02-09

Hot fix release.

### Fixed

* Fix unzip issue when unpacking online save files

## [v0.5.0](https://github.com/bucanero/apollo-ps3/releases/tag/v0.5.0) - 2020-02-07

First public release.

### Added

* Save file listing (+ details)
* Save file SFO unlocking (remove lock flag)
* Save file PFD resigning
* Save file download from Online Database
* Automatic detection of PSID/Account-ID settings
