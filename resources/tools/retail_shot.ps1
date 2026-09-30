param([string]$Out = "shot.png", [string]$ProcName = "Dishonored", [string]$Class = "")
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System; using System.Runtime.InteropServices; using System.Text;
public class S {
 [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
 [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
 [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
 [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
 [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
 [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool att);
 [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
 [StructLayout(LayoutKind.Sequential)] public struct RECT { public int l,t,r,b; }
 [StructLayout(LayoutKind.Sequential)] public struct POINT { public int x,y; }
}
'@
[void][S]::SetProcessDPIAware()
$p = Get-Process $ProcName -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { "NO WINDOW for $ProcName"; exit 1 }
$h = $p.MainWindowHandle
$fg = [S]::GetForegroundWindow()
$a=0; $fgTid=[S]::GetWindowThreadProcessId($fg,[ref]$a); $b=0; $tTid=[S]::GetWindowThreadProcessId($h,[ref]$b)
$me=[S]::GetCurrentThreadId()
[void][S]::AttachThreadInput($me,$fgTid,$true); [void][S]::AttachThreadInput($me,$tTid,$true)
[void][S]::BringWindowToTop($h); [void][S]::SetForegroundWindow($h)
[void][S]::AttachThreadInput($me,$tTid,$false); [void][S]::AttachThreadInput($me,$fgTid,$false)
Start-Sleep -Milliseconds 700
$c = New-Object S+RECT; [void][S]::GetClientRect($h, [ref]$c)
$o = New-Object S+POINT; $o.x=0; $o.y=0; [void][S]::ClientToScreen($h, [ref]$o)
$bmp = New-Object System.Drawing.Bitmap($c.r, $c.b)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($o.x, $o.y, 0, 0, (New-Object System.Drawing.Size($c.r, $c.b)))
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
"SHOT $Out $($c.r)x$($c.b) at ($($o.x),$($o.y)) fg=$([S]::GetForegroundWindow()) target=$h"
