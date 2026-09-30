# Focus the retail Dishonored window, optionally send keys / move+click the mouse, optionally shoot it.
# usage: retail_input.ps1 -Keys "0x0D,0x0D" -Shot out.png -Hover "0.5,0.5" -Click
param(
  [string]$Keys = "",
  [string]$Hover = "",
  [switch]$Click,
  [string]$Shot = "",
  [double]$HoldSeconds = 0.12,
  [double]$AfterSeconds = 1.5,
  [string]$ProcName = "Dishonored"
)

Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class W {
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool attach);
  [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr SetFocus(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr SetActiveWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll", SetLastError=true)] public static extern uint SendInput(uint n, INPUT[] p, int size);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int l, t, r, b; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int x, y; }
  [StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT { public int dx, dy; public uint mouseData, dwFlags, time; public IntPtr extra; }
  [StructLayout(LayoutKind.Sequential)] public struct KEYBDINPUT { public ushort vk, scan; public uint dwFlags, time; public IntPtr extra; }
  [StructLayout(LayoutKind.Explicit)] public struct UNION { [FieldOffset(0)] public MOUSEINPUT m; [FieldOffset(0)] public KEYBDINPUT k; }
  [StructLayout(LayoutKind.Sequential)] public struct INPUT { public uint type; public UNION u; }
}
'@

[void][W]::SetProcessDPIAware()
$proc = Get-Process $ProcName -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $proc) { "NO WINDOW"; exit 1 }
$hwnd = $proc.MainWindowHandle

function Grab-Foreground([IntPtr]$target) {
  # Attaching to the CURRENT foreground window's thread as well as the target's is what makes
  # SetForegroundWindow work while a "Windows Security Alert" dialog owns the foreground. Attaching
  # only to the target's thread is not enough and silently fails.
  $fg = [W]::GetForegroundWindow()
  $a = 0; $fgTid = [W]::GetWindowThreadProcessId($fg, [ref]$a)
  $b = 0; $tTid = [W]::GetWindowThreadProcessId($target, [ref]$b)
  $me = [W]::GetCurrentThreadId()
  [void][W]::AttachThreadInput($me, $fgTid, $true)
  [void][W]::AttachThreadInput($me, $tTid, $true)
  [void][W]::ShowWindow($target, 9)
  [void][W]::BringWindowToTop($target)
  [void][W]::SetForegroundWindow($target)
  [void][W]::SetActiveWindow($target)
  [void][W]::SetFocus($target)
  [void][W]::AttachThreadInput($me, $tTid, $false)
  [void][W]::AttachThreadInput($me, $fgTid, $false)
  Start-Sleep -Milliseconds 400
  return ([W]::GetForegroundWindow() -eq $target)
}

$got = Grab-Foreground $hwnd
if (-not $got) { "WARNING: could not take the foreground" }

function Send-Inputs([W+INPUT[]]$arr) {
  $size = [Runtime.InteropServices.Marshal]::SizeOf([Type]([W+INPUT]))
  $n = [W]::SendInput([uint32]$arr.Length, $arr, $size)
  if ($n -ne $arr.Length) { "SendInput sent $n of $($arr.Length)" }
}

if ($Hover -ne "") {
  $parts = $Hover.Split(",")
  $fx = [double]$parts[0]; $fy = [double]$parts[1]
  $r = New-Object W+RECT
  [void][W]::GetClientRect($hwnd, [ref]$r)
  $p = New-Object W+POINT
  $p.x = [int]($r.r * $fx); $p.y = [int]($r.b * $fy)
  [void][W]::ClientToScreen($hwnd, [ref]$p)
  $vw = [W]::GetSystemMetrics(78); $vh = [W]::GetSystemMetrics(79)
  $ax = [int](65535.0 * $p.x / [Math]::Max(1, $vw - 1))
  $ay = [int](65535.0 * $p.y / [Math]::Max(1, $vh - 1))
  # twelve absolute deltas: DirectInput buffers one report per motion and a single one is droppable
  $list = New-Object System.Collections.Generic.List[W+INPUT]
  for ($i = 1; $i -le 12; $i++) {
    $inp = New-Object W+INPUT
    $inp.type = 0
    $inp.u.m.dx = $ax; $inp.u.m.dy = $ay
    $inp.u.m.dwFlags = 0x0001 -bor 0x8000 -bor 0x4000
    $list.Add($inp)
  }
  Send-Inputs $list.ToArray()
  Start-Sleep -Milliseconds 250
  if ($Click) {
    [void](Grab-Foreground $hwnd)
    $down = New-Object W+INPUT; $down.type = 0; $down.u.m.dwFlags = 0x0002
    $up = New-Object W+INPUT; $up.type = 0; $up.u.m.dwFlags = 0x0004
    Send-Inputs @($down)
    Start-Sleep -Milliseconds 90
    Send-Inputs @($up)
  }
}

if ($Keys -ne "") {
  foreach ($k in $Keys.Split(",")) {
    $vk = [Convert]::ToUInt16($k.Trim(), 16)
    $down = New-Object W+INPUT; $down.type = 1; $down.u.k.vk = $vk
    $up = New-Object W+INPUT; $up.type = 1; $up.u.k.vk = $vk; $up.u.k.dwFlags = 0x0002
    Send-Inputs @($down)
    Start-Sleep -Milliseconds ([int]($HoldSeconds * 1000))
    Send-Inputs @($up)
    Start-Sleep -Milliseconds 400
  }
}

if ($AfterSeconds -gt 0) { Start-Sleep -Milliseconds ([int]($AfterSeconds * 1000)) }

if ($Shot -ne "") {
  $r = New-Object W+RECT
  [void][W]::GetClientRect($hwnd, [ref]$r)
  $p = New-Object W+POINT; $p.x = 0; $p.y = 0
  [void][W]::ClientToScreen($hwnd, [ref]$p)
  $bmp = New-Object System.Drawing.Bitmap($r.r, $r.b)
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen($p.x, $p.y, 0, 0, (New-Object System.Drawing.Size($r.r, $r.b)))
  $bmp.Save($Shot, [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  "SHOT $Shot $($r.r)x$($r.b)"
}
"OK hwnd=$hwnd"
