@echo starting service
@echo off
SysMonitorService.exe /service
net start SysMonitorService
@echo off
@echo end starting service
ping -n 3 127.0.0.1 >nul