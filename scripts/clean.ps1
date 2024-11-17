$current_dir = Get-Location
$root_dir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location -Path $root_dir

$command = "cmake --build build --target clean"
Write-Host $command
Write-Host ""
Invoke-Expression $command

Set-Location -Path $current_dir
