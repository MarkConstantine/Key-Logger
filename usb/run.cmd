@ECHO OFF
SET BASE_DIR="Z:\"
powershell -WindowStyle Hidden -NoProfile -Command "&{ Start-Process powershell -ArgumentList '-WindowStyle Hidden -Execution Bypass -NoProfile -File %BASE_DIR%install_uninstall.ps1' -Verb RunAs}"
