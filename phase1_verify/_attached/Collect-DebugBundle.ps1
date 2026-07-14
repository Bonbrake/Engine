#Requires -Version 5.1
# Collect-DebugBundle.ps1  [M0-EXT-15 audit scaffolding, rewritten clean]
# 14 independent try/catch sections -> manifest.json (Ok/Failed). Read-only.
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Continue'
$RepoRoot = 'C:\ZombieEngine'
$BuildDir = 'build-asan\Release'
$OutDir = Join-Path $PSScriptRoot "debug_bundle_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
$IncludeFocusProbeRun = $true
$ZipOutput = $true
New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
$manifest = @()

function Run-Section($Name, $Body) {
  $path = Join-Path $OutDir "$Name.txt"
  Write-Host "[$Name] collecting..."
  try {
    & $Body 2>&1 | Out-String | Set-Content -Path $path -Encoding UTF8
    $ok = $true; $err = $null
  } catch {
    "ERROR: $($_.Exception.Message)" | Set-Content -Path $path -Encoding UTF8
    $ok = $false; $err = $_.Exception.Message
  }
  $script:manifest += [PSCustomObject]@{ Section = $Name; Ok = $ok; File = $path; Error = $err }
}

Run-Section '01_os' { Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber,OSArchitecture,LastBootUpTime | Format-List; Get-CimInstance Win32_ComputerSystem | Select-Object Manufacturer,Model,SystemType,TotalPhysicalMemory | Format-List }
Run-Section '02_cpu' { Get-CimInstance Win32_Processor | Select-Object Name,NumberOfCores,NumberOfLogicalProcessors,MaxClockSpeed,L2CacheSize,L3CacheSize | Format-List }
Run-Section '03_ram' { Get-CimInstance Win32_PhysicalMemory | ForEach-Object { [PSCustomObject]@{ CapacityGB=[math]::Round($_.Capacity/1GB,1); SpeedMHz=$_.Speed; Manufacturer=$_.Manufacturer; PartNumber=$_.PartNumber.Trim() } } | Format-Table -AutoSize; Get-CimInstance Win32_OperatingSystem | Select-Object @{n='FreeRAM_GB';e={[math]::Round($_.FreePhysicalMemory/1MB,1)}},@{n='TotalRAM_GB';e={[math]::Round($_.TotalVisibleMemorySize/1MB,1)}} }
Run-Section '04_gpu_display' { Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion,DriverDate,AdapterRAM,VideoModeDescription,CurrentRefreshRate,CurrentHorizontalResolution,CurrentVerticalResolution | Format-List; Add-Type -AssemblyName System.Windows.Forms; [System.Windows.Forms.Screen]::AllScreens | ForEach-Object { "Primary=$($_.Primary) Bounds=$($_.Bounds) BitsPerPixel=$($_.BitsPerPixel)" } }
Run-Section '05_disk' { Get-CimInstance Win32_LogicalDisk -Filter "DeviceID='C:'" | Select-Object DeviceID,@{n='SizeGB';e={[math]::Round($_.Size/1GB,1)}},@{n='FreeGB';e={[math]::Round($_.FreeSpace/1GB,1)}}; try { Get-PhysicalDisk | Select-Object DeviceId,MediaType,Size,HealthStatus | Format-Table -AutoSize } catch { 'Get-PhysicalDisk unavailable' } }
Run-Section '06_vulkan' { "VULKAN_SDK=$env:VULKAN_SDK"; $vi = Get-Command vulkaninfo -ErrorAction SilentlyContinue; if ($vi) { & vulkaninfo.exe --summary 2>&1 | Select-String 'apiVersion|driverVersion|deviceName|discrete' } else { 'vulkaninfo.exe not on PATH' } }
Run-Section '07_toolchain' { @('cl','clang-cl','clang','cmake','git','ninja') | ForEach-Object { $c=$_; $p=Get-Command $c -ErrorAction SilentlyContinue; if($p){ & $p.Source --version 2>&1 | Select-Object -First 1 } else { "MISSING: $c" } }; $qs = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'; if (Test-Path $qs) { & $qs -latest -products * -format value -property installationVersion,displayName } else { 'vswhere missing' }; $cc = Join-Path $RepoRoot "build-asan\CMakeCache.txt"; if (Test-Path $cc) { Select-String 'CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER:|CMAKE_GENERATOR:|CMAKE_CXX_FLAGS' $cc } else { 'no CMakeCache.txt' } }
Run-Section '08_asan_dll_fingerprint' {
  $eng = Join-Path $RepoRoot $BuildDir
  $engDll = Join-Path $eng 'clang_rt.asan_dynamic-x86_64.dll'
  "--- ASan DLLs in $engDll ---"
  Get-ChildItem -Path $eng -Filter 'clang_rt.asan*.dll' -ErrorAction SilentlyContinue | ForEach-Object { $v=$_.VersionInfo; "$($_.Name) Size=$($_.Length) FileVersion=$($v.FileVersion) ProductVersion=$($v.ProductVersion) LastWrite=$($_.LastWriteTime)" }
  "--- MSVC-bundled ASan DLL (expected source of truth) ---"
  $qs = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
  if (Test-Path $qs) {
    # Enumerate ALL VS installs (editions/years), not just -latest, so a second
    # install's MSVC toolset isn't silently skipped.
    $vsPaths = & $qs -all -products * -format value -property installationPath 2>$null | Where-Object { $_ }
    if ($vsPaths) {
      $vsPaths | ForEach-Object {
        $p = $_.Trim()
        if ($p) { Get-ChildItem -Path "$p\VC\Tools\MSVC" -Filter 'clang_rt.asan*.dll' -Recurse -ErrorAction SilentlyContinue | Select-Object FullName,Length,LastWriteTime }
      }
    } else { 'vswhere returned no VS installations' }
  } else { 'vswhere missing - cannot locate MSVC toolset root' }
}
Run-Section '09_git_evidence' { Push-Location $RepoRoot; "--- git status --porcelain ---"; git status --porcelain; "--- git log -10 --oneline ---"; git log -10 --oneline; "--- git diff --stat ---"; git diff --stat; "--- git diff (full, unstaged) ---"; git diff; "--- git diff --cached --stat ---"; git diff --cached --stat; Pop-Location }
Run-Section '10_crash_evidence' { "--- Application event log, errors in last 24h ---"; try { Get-WinEvent -FilterHashtable @{LogName='Application';Level=2;StartTime=(Get-Date).AddHours(-24)} -ErrorAction SilentlyContinue | Where-Object { $_.Message -match 'focus_probe|ZombieEngine|TEQ' } | Select-Object TimeCreated,Id,ProviderName,@{n='Msg';e={$_.Message.Substring(0,[Math]::Min(300,$_.Message.Length))}} | Format-List } catch { 'Get-WinEvent query failed or no matches' }; "--- WER local crash dumps ---"; $dd = Join-Path $env:LOCALAPPDATA 'CrashDumps'; if (Test-Path $dd) { Get-ChildItem $dd -Filter '*focus_probe*' -ErrorAction SilentlyContinue | Select-Object Name,Length,LastWriteTime; Get-ChildItem $dd -ErrorAction SilentlyContinue | Select-Object Name,Length,LastWriteTime } else { 'No WER local dump dir' } }
Run-Section '11_overlay_interference_check' { $sus = @('Discord','RTSS','MSIAfterburner','NVIDIA Share','GeForceOverlay','Steam','GameBar','GameBarPresenceWriter','Razer Synapse','WallpaperEngine','EpicGamesLauncher'); Get-Process | Where-Object { $procName = $_.ProcessName; $sus -contains $procName -or ($sus | Where-Object { $procName -like "*$_*" }) } | Select-Object ProcessName,Id,StartTime -ErrorAction SilentlyContinue; "--- Full process list for manual review ---"; Get-Process | Select-Object ProcessName,Id | Sort-Object ProcessName }
Run-Section '12_defender' { try { Get-MpComputerStatus | Select-Object AntivirusEnabled,RealTimeProtectionEnabled,AntivirusSignatureLastUpdated; "--- Exclusions ---"; (Get-MpPreference).ExclusionPath } catch { 'Windows Defender cmdlets unavailable (non-Defender AV, or insufficient privilege)' } }
Run-Section '13_pwsh_runtime' { "PSVersion=$($PSVersionTable.PSVersion)"; "CLR=$($PSVersionTable.CLRVersion)"; try { $dn=Get-Command dotnet -ErrorAction SilentlyContinue; if($dn){ & dotnet --version 2>&1 | Select-Object -First 1 } } catch { 'dotnet not found' } }

if ($IncludeFocusProbeRun) {
  $fp = Join-Path $RepoRoot 'phase1_verify\_attached\Test-FocusProbe.ps1'
  if (Test-Path $fp) {
    Write-Host "Running $fp ..."
    $fpOut = Join-Path $OutDir 'focus_probe_run'
    & $fp -OutDir $fpOut
    $script:manifest += [PSCustomObject]@{ Section='14_focus_probe_run'; Ok=$true; File=$fpOut; Error=$null }
  } else {
    $script:manifest += [PSCustomObject]@{ Section='14_focus_probe_run'; Ok=$false; File=$null; Error="$fp not found" }
  }
}

$manifestPath = Join-Path $OutDir 'manifest.json'
$manifest | ConvertTo-Json -Depth 3 | Set-Content -Path $manifestPath -Encoding UTF8
Write-Host ''
Write-Host '===== COLLECTION SUMMARY =====' -ForegroundColor Cyan
$manifest | Format-Table Section,Ok,Error -AutoSize | Out-String | Write-Host
$failed = $manifest | Where-Object { -not $_.Ok }
if ($failed.Count -gt 0) { Write-Host "$($failed.Count) section(s) failed to collect - see manifest.json" -ForegroundColor Yellow }
if ($ZipOutput) { $zipPath = "$OutDir.zip"; Compress-Archive -Path $OutDir -DestinationPath $zipPath -Force; Write-Host "Bundle zipped: $zipPath" -ForegroundColor Green }
Write-Host "Bundle folder: $OutDir"