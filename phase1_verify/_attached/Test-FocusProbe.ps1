#Requires -Version 5.1
# Test-FocusProbe.ps1  [M0-EXT-15 audit scaffolding — read-only]
# Automated OS-level focus/input regression driver for focus_probe.exe.
# Injects ShowWindowAsync minimize/restore + SendInput Alt+Tab + relative
# mouse motion against a REAL running SDL3 window (live display) — not
# a synthetic/virtual display path. Produces a numeric PASS/FAIL verdict.
Set-StrictMode -Version Latest
param(
  $ExePath,
  $ExeArgs,
  $OutDir,
  $WindowWaitTimeoutSec,
  $SettleDelaySec,
  $MinimizedDurationSec,
  $PostRestoreCaptureSec,
  $AltTabHoldMs,
  $Iterations,
  $DriftThreshold,
  [switch]$SkipAltTab,
  [switch]$InjectMouseBeforeMinimize,
  $HardTimeoutSec
)
$ErrorActionPreference = 'Stop'
if (-not $PSBoundParameters.ContainsKey('ExePath')) { $ExePath = 'build-asan\Release\focus_probe.exe' }
# NOTE: --debug-input-log is NOT a real engine flag (grep across src/ = 0 hits).
# Default to --fly-camera only; the drift CSV check will report INCONCLUSIVE
# until that flag exists — it must never fake a PASS.
if (-not $PSBoundParameters.ContainsKey('ExeArgs')) { $ExeArgs = '--fly-camera' }
if (-not $PSBoundParameters.ContainsKey('OutDir')) { $OutDir = "focus_probe_run_$(Get-Date -Format 'yyyyMMdd_HHmmss')" }
if (-not $PSBoundParameters.ContainsKey('WindowWaitTimeoutSec')) { $WindowWaitTimeoutSec = 15 }
if (-not $PSBoundParameters.ContainsKey('SettleDelaySec')) { $SettleDelaySec = 2 }
if (-not $PSBoundParameters.ContainsKey('MinimizedDurationSec')) { $MinimizedDurationSec = 2 }
if (-not $PSBoundParameters.ContainsKey('PostRestoreCaptureSec')) { $PostRestoreCaptureSec = 3 }
if (-not $PSBoundParameters.ContainsKey('AltTabHoldMs')) { $AltTabHoldMs = 150 }
if (-not $PSBoundParameters.ContainsKey('Iterations')) { $Iterations = 1 }
if (-not $PSBoundParameters.ContainsKey('DriftThreshold')) { $DriftThreshold = 0.01 }
if (-not $PSBoundParameters.ContainsKey('HardTimeoutSec')) { $HardTimeoutSec = 60 }

New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
$logFile = Join-Path $OutDir 'run.log'

function Write-Log($Msg, $Level='INFO') {
  $line = '[{0}] [{1}] {2}' -f (Get-Date -Format 'HH:mm:ss.fff'), $Level, $Msg
  Write-Host $line
  Add-Content -Path $logFile -Value $line
}

$csharp = @'
using System;
using System.Runtime.InteropServices;
public static class Win32Input {
  [DllImport("user32.dll")] public static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr hWnd);
  [DllImport("user32.dll", SetLastError=true)] public static extern uint SendInput(uint nInputs, INPUT[] pInputs, int cbSize);
  [DllImport("user32.dll")] public static extern bool AllowSetForegroundWindow(uint dwProcessId);
  public const int SW_MINIMIZE = 6;
  public const int SW_RESTORE = 9;
  public const int INPUT_MOUSE = 0;
  public const int INPUT_KEYBOARD = 1;
  public const uint KEYEVENTF_KEYUP = 0x0002;
  public const uint MOUSEEVENTF_MOVE = 0x0001;
  public const ushort VK_MENU = 0x12;
  public const ushort VK_TAB = 0x09;
  [StructLayout(LayoutKind.Sequential)] public struct KEYBDINPUT { public ushort wVk; public ushort wScan; public uint dwFlags; public uint time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT { public int dx; public int dy; public uint mouseData; public uint dwFlags; public uint time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Sequential)] public struct HARDWAREINPUT { public uint uMsg; public ushort wParamL; public ushort wParamH; }
  [StructLayout(LayoutKind.Explicit)] public struct INPUTUNION { [FieldOffset(0)] public MOUSEINPUT mi; [FieldOffset(0)] public KEYBDINPUT ki; [FieldOffset(0)] public HARDWAREINPUT hi; }
  [StructLayout(LayoutKind.Sequential)] public struct INPUT { public uint type; public INPUTUNION u; }
}
'@
Add-Type -TypeDefinition $csharp -Language CSharp
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$t = [Win32Input+INPUT]::New()
$inputSize = [System.Runtime.InteropServices.Marshal]::SizeOf($t)

function New-KeyInput($Vk, $KeyUp) {
  $i = New-Object Win32Input+INPUT
  $i.type = [Win32Input]::INPUT_KEYBOARD
  $i.u.ki.wVk = $Vk
  $i.u.ki.dwFlags = if ($KeyUp) { [Win32Input]::KEYEVENTF_KEYUP } else { 0 }
  $i.u.ki.dwExtraInfo = [IntPtr]::Zero
  return $i
}

function Send-AltTab($HoldMs=150) {
  Write-Log "SendInput: Alt+Tab (hold ${HoldMs}ms)"
  $down = @( (New-KeyInput -Vk ([Win32Input]::VK_MENU) -KeyUp $false),
             (New-KeyInput -Vk ([Win32Input]::VK_TAB)  -KeyUp $false) )
  [Win32Input]::SendInput(2, $down, $inputSize) | Out-Null
  Start-Sleep -Milliseconds $HoldMs
  [Win32Input]::SendInput(1, @(New-KeyInput -Vk ([Win32Input]::VK_TAB) -KeyUp $true), $inputSize) | Out-Null
  Start-Sleep -Milliseconds 50
  [Win32Input]::SendInput(1, @(New-KeyInput -Vk ([Win32Input]::VK_MENU) -KeyUp $true), $inputSize) | Out-Null
}

function Send-MouseMove($Dx, $Dy) {
  Write-Log "SendInput: mouse move dx=$Dx dy=$Dy"
  $i = New-Object Win32Input+INPUT
  $i.type = [Win32Input]::INPUT_MOUSE
  $i.u.mi.dx = $Dx
  $i.u.mi.dy = $Dy
  $i.u.mi.dwFlags = [Win32Input]::MOUSEEVENTF_MOVE
  $i.u.mi.dwExtraInfo = [IntPtr]::Zero
  [Win32Input]::SendInput(1, @($i), $inputSize) | Out-Null
}

function Wait-ForMainWindowHandle($Process, $TimeoutSec=15) {
  $sw = [System.Diagnostics.Stopwatch]::StartNew()
  while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec) {
    $Process.Refresh()
    if ($Process.HasExited) { throw "Process exited before creating a window (exit code $($Process.ExitCode))." }
    if ($Process.MainWindowHandle -ne [IntPtr]::Zero) { return $Process.MainWindowHandle }
    Start-Sleep -Milliseconds 200
  }
  throw "Timed out after ${TimeoutSec}s waiting for main window handle."
}

function Wait-ForIconicState($Hwnd, $ExpectIconic, $TimeoutSec=5) {
  $sw = [System.Diagnostics.Stopwatch]::StartNew()
  while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec) {
    if ([Win32Input]::IsIconic($Hwnd) -eq $ExpectIconic) { return $true }
    Start-Sleep -Milliseconds 100
  }
  return $false
}

function Save-Screenshot($Path) {
  try {
    $bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $bmp = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
    $bmp.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    Write-Log "Screenshot saved: $Path"
  } catch {
    Write-Log "Screenshot FAILED (no display session?): $($_.Exception.Message)" 'WARN'
  }
}

# ASSUMED CSV format - see header doc comment. NOT verified against real
# Input.cpp output. The engine has NO --debug-input-log flag (grep across
# src/ returned 0 hits; real flags are --headless/--fly-camera/--quit-frame/
# --script-input/--dump-state). Test-CameraDrift will return INCONCLUSIVE
# until that flag exists. Do NOT trust a drift verdict without the flag.
function Test-CameraDrift($LogPath, $Threshold) {
  if (-not (Test-Path $LogPath)) {
    return [PSCustomObject]@{
      Available = $false
      Verdict = "INCONCLUSIVE - no delta log at $LogPath (flag not implemented, or path wrong)"
      MaxYaw = $null; MaxPitch = $null; ViolatingFrames = $null
    }
  }
  try {
    $rows = Import-Csv -Path $LogPath -Header frame, dt, yaw, pitch |
      ForEach-Object { [PSCustomObject]@{
        frame = [int]$_.frame
        dt    = [double]$_.dt
        yaw   = [double]$_.yaw
        pitch = [double]$_.pitch
      } }
  } catch {
    return [PSCustomObject]@{
      Available = $false
      Verdict = "INCONCLUSIVE - log at $LogPath did not parse as frame,dt,yaw,pitch CSV: $($_.Exception.Message)"
      MaxYaw = $null; MaxPitch = $null; ViolatingFrames = $null
    }
  }
  if ($rows.Count -eq 0) {
    return [PSCustomObject]@{
      Available = $true
      Verdict = "INCONCLUSIVE - log present but empty"
      MaxYaw = $null; MaxPitch = $null; ViolatingFrames = $null
    }
  }
  $maxYaw   = ($rows | ForEach-Object { [math]::Abs($_.yaw) }   | Measure-Object -Maximum).Maximum
  $maxPitch = ($rows | ForEach-Object { [math]::Abs($_.pitch) } | Measure-Object -Maximum).Maximum
  $violating = $rows | Where-Object { [math]::Abs($_.yaw) -gt $Threshold -or [math]::Abs($_.pitch) -gt $Threshold }
  $verdict = if ($violating.Count -eq 0) { "PASS - no post-restore frame exceeded drift threshold ($Threshold)" } else { "FAIL - $($violating.Count)/$($rows.Count) frames exceeded drift threshold ($Threshold); camera likely stuck drifting" }
  return [PSCustomObject]@{
    Available = $true
    Verdict = $verdict
    MaxYaw = $maxYaw
    MaxPitch = $maxPitch
    ViolatingFrames = $violating.Count
    TotalFrames = $rows.Count
  }
}

# Main iteration loop
$results = @()
for ($iter = 1; $iter -le $Iterations; $iter++) {
  Write-Log "===== Iteration $iter / $Iterations =====" 'ITER'
  $iterDir = Join-Path $OutDir "iter_$iter"
  New-Item -ItemType Directory -Path $iterDir -Force | Out-Null
  $stdoutPath = Join-Path $iterDir 'stdout.txt'
  $deltaLogPath = Join-Path $iterDir 'input_delta_log.csv'
  $iterResult = [PSCustomObject]@{
    Iteration = $iter
    Launched = $false
    HandleAcquired = $false
    MinimizeOk = $false
    RestoreOk = $false
    AltTabRan = $false
    ExitCode = $null
    Crashed = $false
    Killed = $false
    SdlFail = $false
    Drift = $null
    Error = $null
  }
  $proc = $null
  try {
    Write-Log "Launching: $ExePath $ExeArgs"
    $proc = Start-Process -FilePath $ExePath -ArgumentList $ExeArgs -RedirectStandardOutput $stdoutPath -PassThru
    $iterResult.Launched = $true
    $hwnd = Wait-ForMainWindowHandle -Process $proc -TimeoutSec $WindowWaitTimeoutSec
    $iterResult.HandleAcquired = $true
    Write-Log "Window handle acquired: 0x$($hwnd.ToString('X'))"
    Start-Sleep -Seconds $SettleDelaySec
    Save-Screenshot -Path (Join-Path $iterDir '01_launch.png')
    if ($InjectMouseBeforeMinimize) {
      Send-MouseMove -Dx 40 -Dy 0
      Start-Sleep -Milliseconds 200
    }
    Write-Log "Minimizing (ShowWindowAsync SW_MINIMIZE)"
    [Win32Input]::ShowWindowAsync($hwnd, [Win32Input]::SW_MINIMIZE) | Out-Null
    $iterResult.MinimizeOk = Wait-ForIconicState -Hwnd $hwnd -ExpectIconic $true -TimeoutSec 5
    if (-not $iterResult.MinimizeOk) { Write-Log "WARNING: window did not report iconic state within timeout" 'WARN' }
    Start-Sleep -Seconds $MinimizedDurationSec
    Write-Log "Restoring (ShowWindowAsync SW_RESTORE)"
    [Win32Input]::ShowWindowAsync($hwnd, [Win32Input]::SW_RESTORE) | Out-Null
    $iterResult.RestoreOk = Wait-ForIconicState -Hwnd $hwnd -ExpectIconic $false -TimeoutSec 5
    [Win32Input]::SetForegroundWindow($hwnd) | Out-Null
    if (-not $iterResult.RestoreOk) { Write-Log "WARNING: window did not leave iconic state within timeout" 'WARN' }
    Save-Screenshot -Path (Join-Path $iterDir '02_post_restore.png')
    if (-not $SkipAltTab) {
      Send-AltTab -HoldMs $AltTabHoldMs
      Start-Sleep -Milliseconds 500
      [Win32Input]::SetForegroundWindow($hwnd) | Out-Null
      $iterResult.AltTabRan = $true
      Save-Screenshot -Path (Join-Path $iterDir '03_post_alttab.png')
    }
    Write-Log "Capturing $PostRestoreCaptureSec s of post-restore behavior for drift analysis"
    Start-Sleep -Seconds $PostRestoreCaptureSec
    $exited = $proc.WaitForExit($HardTimeoutSec * 1000)
    if (-not $exited) {
      Write-Log "Process did not exit within ${HardTimeoutSec}s - killing" 'WARN'
      Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
      $iterResult.Killed = $true
    } else {
      $iterResult.ExitCode = $proc.ExitCode
      $iterResult.Crashed = ($proc.ExitCode -ne 0)
    }
  } catch {
    $iterResult.Error = $_.Exception.Message
    Write-Log "ERROR: $($_.Exception.Message)" 'ERROR'
    if ($proc -and -not $proc.HasExited) {
      Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
      $iterResult.Killed = $true
    }
  } finally {
    if ($proc -and -not $proc.HasExited) {
      Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    }
  }
  $iterResult.Drift = Test-CameraDrift -LogPath $deltaLogPath -Threshold $DriftThreshold
  # A probe that never obtained a display (SDL_Init FAIL) cannot satisfy the
  # Step 4c hardware PASS bar - record per-iteration, never a silent PASS.
  if (Test-Path $stdoutPath) { $iterResult.SdlFail = (Get-Content $stdoutPath -Raw) -match 'SDL_Init FAIL' }
  Write-Log ("Iteration {0} drift verdict: {1}" -f $iter, $iterResult.Drift.Verdict)
  if (Test-Path $stdoutPath) {
    Write-Log "---- stdout tail (iter $iter) ----"
    Get-Content $stdoutPath -Tail 20 | ForEach-Object { Write-Log $_ 'STDOUT' }
  }
  $results += $iterResult
}

# Summary
$summaryPath = Join-Path $OutDir 'summary.json'
$results | ConvertTo-Json -Depth 5 | Set-Content -Path $summaryPath
Write-Log "Summary written: $summaryPath"

Write-Host ''
Write-Host '===== RUN SUMMARY =====' -ForegroundColor Cyan
$results | Format-Table Iteration, Launched, HandleAcquired, MinimizeOk, RestoreOk, AltTabRan, ExitCode, Crashed, Killed,
  @{Label='Drift'; Expression={$_.Drift.Verdict}} -AutoSize | Out-String | Write-Host

$methodologyNote = @'
## Methodology note ($(Get-Date -Format 'yyyy-MM-dd HH:mm'))
This test run used automated OS-level input (ShowWindowAsync + SendInput) on real
hardware with a live display - not a virtual/synthetic display path
(distinct from the earlier M0:EXT15-runtime-virtual Xvfb exclusion). Unattended,
not manual. See Test-FocusProbe.ps1.
'@
Add-Content -Path (Join-Path $OutDir 'STATUS_note.md') -Value $methodologyNote

# A probe that never obtained a display (SDL_Init FAIL) cannot satisfy the
# Step 4c hardware PASS bar - treat as a hard failure, never a silent PASS.
$failures = @($results | Where-Object {
  $_.Error -or $_.Crashed -or $_.Killed -or -not $_.MinimizeOk -or -not $_.RestoreOk -or
  $_.SdlFail -or
  ($_.Drift.Available -and $_.Drift.Verdict -like 'FAIL*')
})
if ($failures.Count -gt 0) {
  Write-Host "$($failures.Count)/$Iterations iteration(s) FAILED." -ForegroundColor Red
  exit 1
} else {
  Write-Host "All $Iterations iteration(s) PASSED." -ForegroundColor Green
  exit 0
}
