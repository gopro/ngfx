# Install VSSetup module if not existing
if ((Get-InstalledModule -Name "VSSetup" -ErrorAction SilentlyContinue) -eq $null) {
    Install-Module VSSetup -Scope CurrentUser -Force
}

# Get vcvars64 of latest VS installation instance
$vsInstallationInfo = Get-VSSetupInstance -All -Prerelease |
    Sort-Object -Property InstallationVersion -Descending |
    Select-Object -First 1
$vcvars64 = "$($vsInstallationInfo.InstallationPath)\VC\Auxiliary\Build\vcvars64.bat"

Write-Host $vcvars64
