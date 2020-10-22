@echo off

REM # Powershell hidden window execution of VM Detection Script

powershell -executionpolicy bypass -nop -window hidden $env:Temp\checkV.bat