
cd "C:\Users\97254\esp_projects\blink_led"
idf.py set-target esp32
idf.py fullclean
idf.py reconfigure
idf.py build
idf.py -p COM4 flash
