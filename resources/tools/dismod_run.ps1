# Launch retail Dishonored with the dismod probe. See resources/docs/dismod_harness.md.
#   .\dismod_run.ps1 -Agent FB [-Hooks "advance"] [-Wait 55] [-Fresh]
param([string]$Agent = "FB", [string]$Hooks = "", [int]$Wait = 55, [switch]$Fresh, [switch]$NoDeploy)
$game = "C:\Program Files (x86)\Steam\steamapps\common\Dishonored\Binaries\Win32"
$R = "D:\RecompileDishonored\Recompile\build\agent$Agent\retail"
New-Item -ItemType Directory -Force -Path $R | Out-Null
if (-not $NoDeploy) { Copy-Item "D:\Christmas\github\dismod\build\debug\dismod.dll" (Join-Path $game "dinput8.dll") -Force }
if ($Fresh) { Get-ChildItem "$R\*.log" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue }
Set-Content -Path "$R\probe_cmd.txt" -Value "# probe commands" -Encoding ascii
$env:DISMOD_UIPROBE = "1"
$env:DISMOD_PROBE_DIR = $R
$env:DISMOD_LOG = "D:\RecompileDishonored\Recompile\build\agent$Agent\dismod.log"
if ($Hooks -ne "") { $env:DISMOD_UIPROBE_HOOKS = $Hooks } else { Remove-Item Env:\DISMOD_UIPROBE_HOOKS -ErrorAction SilentlyContinue }
Start-Process -FilePath (Join-Path $game "Dishonored.exe") -WorkingDirectory $game `
  -ArgumentList "-windowed","-ResX=1600","-ResY=900","-nostartupmovies"
Start-Sleep -Seconds $Wait
$p = Get-Process Dishonored -ErrorAction SilentlyContinue
if ($p) { "ALIVE: " + ($p.Id -join ",") } else { "DEAD" }
