$Process = Start-Process -FilePath "C:\ZombieEngine\build\Debug\ZombieEngine.exe" -PassThru
Start-Sleep -Seconds 5
Stop-Process -Id $Process.Id -Force
