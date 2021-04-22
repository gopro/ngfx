if (Test-Path env:VCVARS64) {
    Exit
}

# Install VSSetup module if not existing
if ((Get-InstalledModule -Name "VSSetup" -ErrorAction SilentlyContinue) -eq $null) {
    Install-Module VSSetup -Scope CurrentUser -Force
}

# Get vcvars64 of latest VS installation instance
$vsInstallationInfo = Get-VSSetupInstance -All -Prerelease |
    Sort-Object -Property InstallationVersion -Descending |
    Select-Object -First 1
$vcvars64 = "$($vsInstallationInfo.InstallationPath)\VC\Auxiliary\Build\vcvars64.bat"

cmd /c "call `"$vcvars64`" && set" |
foreach {
    if ($_ -match "=") {
        $v = $_.split("="); Set-Item -force -Path "env:\$($v[0])" -Value "$($v[1])"
    }
}

Set-Item -force -Path env:VCVARS64 -Value "$vcvars64"
