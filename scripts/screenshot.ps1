Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
$Process = Start-Process -FilePath "C:\ZombieEngine\build\Debug\ZombieEngine.exe" -PassThru
Start-Sleep -Seconds 3
$Screen = [System.Windows.Forms.SystemInformation]::VirtualScreen
$Bitmap = New-Object System.Drawing.Bitmap $Screen.Width, $Screen.Height
$Graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
$Graphics.CopyFromScreen($Screen.Left, $Screen.Top, 0, 0, $Bitmap.Size)
$Bitmap.Save("C:\Users\jakeb\.gemini\antigravity\brain\f182278b-eaff-4699-8df1-d7fd8a2af250\screenshot.png")
$Graphics.Dispose()
$Bitmap.Dispose()
Stop-Process -Id $Process.Id -Force
