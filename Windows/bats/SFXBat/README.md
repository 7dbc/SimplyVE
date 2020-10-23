# SFXBat
# Authered by b1krb7

Simple bat script, combined with a base64 encoded Powershell Downloader.
Once the SFX Archive has been downloaded to the Temp Directory, the
SFX Archive is then executed unzipping the archive and allowing execution
of the first bat archive allowing a hidden powershell window to execute the
second bat script hidden the window while it runs through VM Detection strings.
While removing all previously run scripts removing traces of infection.

If a VM is detected all files are then removed.
If no VM is detected the payload will be launched and upon runtime
deleted leaving no file traces for infection analysis.

# Important
[!] Change that names of the scripts and file names within the runtime scripts.


Contents:
- PAYLOAD.zip.exe - Benign SFX Archive to show placement of files for production usage.
<br />'-> Included to show the SFX options to control how the archive is handled upon execution.
- preV.bat - Bat script to run the VM Detection Script from a hidden window.
- checkV.bat - The Script that checks for signs of virtualization and cleans the environment during execution.
- SFX_Downloader.ps1 - The base64 encoded powershell script that pulls and executes the SFXArchive from WebServer.
- PAYLOADTOEXECUTE.exe.txt - This would be the malware file to run if machine comes back clean.


<details>
<summary>Required Structure</summary>
[1] SFX_Downloader.ps1

[2] PAYLOAD.zip.exe<br />
&emsp;    ├── PAYLOADTOEXECUTE.exe<br />
&emsp;    ├── preV.bat<br />
&emsp;    ├── checkV.bat<br />
&emsp;    └── ...<br />
</details>
