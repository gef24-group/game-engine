param(
    [string] $PROFILE = "OFF"
)

$current_dir = Get-Location
$root_dir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location -Path $root_dir

$command = "cmake -G Ninja -S . -B build -DPROFILE=$PROFILE"
Write-Host $command
Write-Host ""
Invoke-Expression $command

Set-Location -Path $current_dir
