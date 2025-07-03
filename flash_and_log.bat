@echo off
:: Activate ESP-IDF
call "C:\Espressif\esp-idf\export.bat"

:: Run the flash command and save output to log
idf.py -p COM4 flash > flash_output.txt

:: Run the Python script to parse the log and launch TeraTerm
python "C:\Users\97254\esp_projects\open_teraterm_if_done.py"
pause