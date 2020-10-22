# SFXBat
# Authered by B1kRb7

Simple bat script, combined with a base64 encoded Powershell Downloader.
Once the SFX Archive has been downloaded to the Temp Directory, the
SFX Archive is then executed unzipping the archive and allowing execution
of the first bat archive allowing a hidden powershell window to execute the
second bat script hidden the window while it runs through VM Detection strings.
While removing all previously run scripts removing traces of infection.

If a VM is detected all files are then removed.
If no VM is detected the payload will be launched and upon runtime
deleted leaving no file traces for infection analysis.