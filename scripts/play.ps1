param(
    [string] $GAME = "all",
    [string] $GAME_ARGS = "",
    [string] $TRACY_PORT = "9000"
)

$current_dir = Get-Location
$root_dir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location -Path $root_dir

if ($GAME -ceq "all") {
    Write-Host "Usage:"
    Write-Host "play.ps1 -GAME <game> -GAME_ARGS <game_args> [ -TRACY_PORT <port> ]"
    Write-Host ""
    Write-Host "Where:"
    Write-Host "<game> is one of:"
    Get-Content -Path ".targetgames"
    Write-Host ""

    Set-Location -Path $current_dir
    exit 1
}

$command = "./build/$GAME/$GAME $GAME_ARGS"
Write-Host $command
Write-Host ""
$env:TRACY_PORT = $TRACY_PORT
Invoke-Expression $command

Set-Location -Path $current_dir
