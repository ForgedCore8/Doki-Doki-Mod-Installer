---
<div align=center>

# DDMI (Doki Doki Mod Installer)

[Join the Discord!](https://discord.gg/c8XV2BuA)

![GitHub all releases](https://img.shields.io/github/downloads/ForgedCore8/doki-doki-mod-installer/total) ![GitHub](https://img.shields.io/github/license/ForgedCore8/doki-doki-mod-installer)
</div>

DDMI is a tool designed to streamline the installation of mods for "Doki Doki Literature Club!" (DDLC). It simplifies the process, making it accessible to those who may not be familiar with the manual installation process. By automating the detection of the game directory and the extraction and placement of mod files, DDMI ensures that mods are installed correctly and efficiently.

## Features

- **Automatic Game Directory Detection:** Automatically locates your DDLC installation directory for ease of mod installation.
- **Mod Installation:** Easily install mods from a ZIP file with a single click.
- **Progress Monitoring:** Includes a progress bar to monitor the installation process.
- **Safety Checks:** Performs checks to ensure that the targeted directory is a valid DDLC installation to prevent accidental file deletion or modification.
- **Clean Uninstallation:** Offers an option to delete the DDLC directory, ensuring a clean slate for mod installation.
- **Console Output:** Provides detailed console output for troubleshooting and process tracking.

## Installation

To use DDMI, you'll need a Windows system with Python installed. Follow these steps:

1. Ensure that your Antivirus allows DDMI
2. Download the DDMI script or binary from the repository.
3. Double-click the script to run the DDMI interface.

*Note: DDMI is currently only available for Windows users.*

## Usage

1. **Start DDMI:** Open the DDMI tool.
2. **Select ZIP File:** Click on 'Browse' to select the mod file (in ZIP format) you wish to install.
3. **Select Game Directory:** Use the 'Browse' button to select your DDLC game directory. You can also use the 'Auto' button to automatically detect your game directory.
4. **Install Mod:** Click on 'Install Mod' to begin the installation process. Follow the on-screen instructions to complete the installation.
5. **Console Output:** Monitor the console output for process updates and potential error messages.

## Uninstallation

To uninstall a mod, simply use the 'Delete DDLC' button to remove the entire DDLC directory. It is recommended to reinstall a fresh copy of DDLC before installing a new mod.

## Troubleshooting

If you encounter issues during installation:
- Ensure that the ZIP file is not corrupted and is a valid DDLC mod.
- Verify that the game directory is correctly selected and corresponds to a valid DDLC installation.
- Check the console output for error messages that can provide more insight into the issue.

For additional help, please refer to the community forums or the mod's documentation.


## Compiling
To Compile your own copy of DDMI, you can run the following commands:

```pip install nuitka```

```nuitka --enable-plugin=pyside6 --follow-imports --windows-disable-console --include-data-dir=./assets=./assets ddmi.py```


## Contributing

Contributions to DDMI are welcome! If you have suggestions for improvements or bug fixes, please open an issue or submit a pull request.

## License

DDMI is open-source software licensed under GNU GPL v3.0. Please review the license terms before modifying or redistributing the software.

## Acknowledgments

Thanks to the DDLC community for their continuous support and creativity in mod development.

---
