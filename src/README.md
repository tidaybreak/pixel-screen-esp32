ESP32-HUB75-MatrixPanel-DMA 2.0.7版本或以后的版本如下修改
Documents\PlatformIO\Projects\230708-162713-esp32dev\.pio\libdeps\esp32dev\ESP32 HUB75 LED MATRIX PANEL DMA Display\src\platforms\esp32\esp32_i2s_parallel_dma.cpp 
    dev->sample_rate_conf.rx_bck_div_num = 6;
    dev->sample_rate_conf.tx_bck_div_num = 6;



[env:esp32dev]
platform = espressif32
;board = esp32dev
board = esp-wrover-kit
framework = arduino
monitor_speed = 115200
;monitor_port = COM7
board_build.partitions = partitions.csv
lib_deps = 
	bblanchon/ArduinoJson@^6.21.2
	paulstoffregen/Time@^1.6.1
	winlinvip/SimpleDHT@^1.0.15
	adafruit/Adafruit GFX Library@^1.11.5
	marian-craciunescu/ESP32Ping@^1.7
	toblum/TetrisAnimation@^1.1.0
	2dom/PxMatrix LED MATRIX library@^1.8.2
	ropg/ezTime@^0.8.3
	mrfaptastic/ESP32 HUB75 LED MATRIX PANEL DMA Display@^3.0.9


PS C:\Espressif\frameworks\esp-idf-v5.1-2> openocd -f board/esp32-wrover-kit-3.3v.cfg