# IMUtest 與 sdkTest 資料流說明

這份文件整理 `/home/chc/code/IMUtest` 與 `/home/chc/code/sdkTest` 兩個資料夾的角色、相互關係、資料流、啟動流程，以及後續若要接 KNEO Pi 或 UART-to-CAN 時需要注意的地方。

## 一句話總結

目前系統可以理解成：

```text
兩顆 IMU ESP32 + 一顆 CAN/CCPA ESP32-C6
        ↓ USB Serial / UART
電腦端同時開三個 logger
        ↓
同一個 run 資料夾中的三份 CSV
```

`IMUtest` 主要負責 MPU6050 六軸 IMU 資料；`sdkTest` 主要負責 CAN bus / CCPA telemetry 資料。真正把三路資料一起收進 CSV 的啟動腳本在 `IMUtest/tools/record_three_esp32.sh`。

## 兩個資料夾各自在做什麼

### IMUtest

`IMUtest` 目前比較像是「IMU 節點專案」。

重要檔案：

```text
IMUtest/platformio.ini
IMUtest/src/main.cpp
IMUtest/src/imu_logger.cpp
IMUtest/src/imu_logger.h
IMUtest/tools/serial_csv_logger.cpp
IMUtest/tools/record_three_esp32.sh
IMUtest/tools/monitor_csv_logger.sh
IMUtest/tools/record_imu.py
IMUtest/tools/imu_live_dashboard.html
```

`IMUtest/platformio.ini` 目前設定：

```ini
[env:esp32dev]
board = esp32dev
framework = arduino
monitor_speed = 115200
build_flags =
    -D CAN_lib_1
    -DENABLE_IMU_LOGGER=1
    -DENABLE_CAN_BRIDGE=0
```

這代表它現在燒錄出來的 firmware 是：

- 開啟 IMU logger。
- 關閉 CAN bridge。
- 使用 ESP32 dev board。
- Serial baud rate 是 115200。

`IMUtest/src/main.cpp` 在 `setup()` 裡會呼叫：

```cpp
imu_logger_begin(pinMcu_IMU_SDA, pinMcu_IMU_SCL);
```

在 `loop()` 裡會呼叫：

```cpp
imu_logger_update();
```

所以 `IMUtest` 現在的主功能是：ESP32 透過 I2C 讀 MPU6050，然後用 Serial 輸出 CSV 格式的 IMU 資料。

### sdkTest

`sdkTest` 目前比較像是「CAN/CCPA telemetry 節點專案」。

重要檔案：

```text
sdkTest/platformio.ini
sdkTest/src/main.cpp
sdkTest/src/ccpa_telemetry.c
sdkTest/src/ccpa_telemetry.h
sdkTest/src/CAN_base.cpp
sdkTest/src/CAN_base.h
sdkTest/src/s_canbus_spi.cpp
sdkTest/src/s_canbus_spi.h
sdkTest/tools/serial_csv_logger.cpp
sdkTest/tools/monitor_csv_logger.sh
sdkTest/tools/ccpa_telemetry_dashboard.html
sdkTest/tools/csv_telemetry_dashboard.html
sdkTest/tools/serial_web_dashboard.py
```

`sdkTest/platformio.ini` 目前設定：

```ini
[env:esp32-c6-devkitm-1]
board = esp32-c6-devkitm-1
framework = arduino
monitor_speed = 115200
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
    -D CAN_lib_1
```

這代表它現在燒錄出來的 firmware 是：

- 使用 ESP32-C6 devkit。
- 開啟 USB CDC，讓板子可以透過 USB 直接當 Serial port。
- 使用 ESP32 TWAI CAN driver。
- Serial baud rate 是 115200。

`sdkTest/src/main.cpp` 會初始化兩種 CAN 接口：

```cpp
CAN_base_setPin(pinMcu_CANBUS_RX_2, pinMcu_CANBUS_TX_2, 250000);
canbus_spi_init();
CAN_base_init();
```

也就是：

- `CAN_base_*`：ESP32 內建 TWAI CAN controller。
- `canbus_spi_*`：外接 MCP2515 SPI CAN controller。

兩邊 CAN bitrate 都是以 250 kbps 為主。`s_canbus_spi.cpp` 中 MCP2515 使用 `CAN_250KBPS` 與 `MCP_16MHZ`。

## 兩者相互關係

這兩個專案不是彼此直接呼叫函式，而是透過「電腦端啟動腳本」被整合在一起。

```text
IMUtest firmware A  -> /dev/ttyUSB0 -> IMU CSV logger
IMUtest firmware B  -> /dev/ttyUSB1 -> IMU CSV logger
sdkTest firmware    -> /dev/ttyACM0 -> telemetry CSV logger
```

整合點是：

```text
IMUtest/tools/record_three_esp32.sh
```

這支腳本會同時啟動三個電腦端 logger：

```text
IMU A     使用 IMUtest/tools/serial_csv_logger
IMU B     使用 IMUtest/tools/serial_csv_logger
Telemetry 使用 sdkTest/tools/serial_csv_logger
```

也就是說，`IMUtest` 不只放 IMU firmware，也放了「一次收三路資料」的總啟動腳本；`sdkTest` 則提供 CAN/CCPA firmware 與 CAN telemetry 專用 CSV logger。

## 硬體資料流

整體硬體概念如下：

```text
MPU6050 A
   ↓ I2C
ESP32 A running IMUtest firmware
   ↓ USB Serial, 115200
電腦 / KNEO Pi / Linux 主機

MPU6050 B
   ↓ I2C
ESP32 B running IMUtest firmware
   ↓ USB Serial, 115200
電腦 / KNEO Pi / Linux 主機

CAN bus
   ↓ CANH / CANL
ESP32-C6 running sdkTest firmware
   ↓ USB Serial, 115200
電腦 / KNEO Pi / Linux 主機
```

電腦端再把三條 Serial 資料各自寫入 CSV。

## IMU 資料流

### 1. ESP32 端

`IMUtest/src/imu_logger.cpp` 會：

1. 啟動 I2C。
2. 掃描 MPU6050 位址 `0x68` 或 `0x69`。
3. 設定 MPU6050：
   - 加速度範圍：`+/- 8 g`
   - 陀螺儀範圍：`+/- 500 deg/s`
   - sample rate：約 100 Hz
4. 開機後做 gyro bias calibration。
5. 透過 Serial 印出 CSV header。
6. 每 10 ms 輸出一筆 IMU 資料。

輸出格式：

```csv
type,t_ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,temp_c,accel_norm_g
IMU,275641,0.03833,0.94116,0.41089,-0.056,0.440,-0.031,25.01,1.02766
```

欄位意思：

```text
type          固定是 IMU
t_ms          ESP32 自己的 millis() 時間
ax_g          X 軸加速度，單位 g
ay_g          Y 軸加速度，單位 g
az_g          Z 軸加速度，單位 g
gx_dps        X 軸角速度，degree per second
gy_dps        Y 軸角速度，degree per second
gz_dps        Z 軸角速度，degree per second
temp_c        MPU6050 溫度，攝氏
accel_norm_g  三軸加速度向量大小
```

### 2. 電腦端

`IMUtest/tools/serial_csv_logger.cpp` 會讀 Serial port，只接受開頭是：

```text
IMU,
```

的行，然後加上一個主機時間 `host_time_s`，寫成：

```csv
host_time_s,type,t_ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,temp_c,accel_norm_g
1779954963.067,IMU,275641,0.03833,0.94116,0.41089,-0.056,0.440,-0.031,25.01,1.02766
```

輸出檔名會自動遞增：

```text
imu_test_001.csv
imu_test_002.csv
...
```

## CAN/CCPA telemetry 資料流

### 1. ESP32-C6 端

`sdkTest/src/main.cpp` 會做幾件事：

1. 初始化 Serial。
2. 初始化 CAN queue。
3. 初始化 CCPA telemetry state。
4. 初始化 MCP2515 SPI CAN。
5. 初始化 ESP32 TWAI CAN。
6. 在 loop 中持續收 CAN frame。
7. 對收到的 frame 呼叫 `ccpa_telemetry_process_frame()`。
8. 若解析出 telemetry 變化，就用 Serial 印出人類可讀的 log。

典型 Serial log 會長得像：

```text
RAW RX TWAI ID=0x01E942446 DLC=8 DATA=...
CCPA telemetry update ID=0x01E942446 changed=0x00000038
Battery SOC = 98 %
Battery voltage = 47810 mV
Battery current = 70 mA
```

`sdkTest/src/ccpa_telemetry.c` 目前會解析的資料包含：

```text
bike speed
cadence
rider torque
motor RPM
motor temperature
assist level / riding mode
battery SOC
battery voltage
battery current
battery temperatures
rear gear
```

另外 `sdkTest/src/main.cpp` 會定期送 battery temperature request：

```cpp
requestBatteryTemps();
```

目前間隔是 1000 ms。

### 2. 電腦端

`sdkTest/tools/serial_csv_logger.cpp` 不是直接吃 binary CAN frame，而是讀 ESP32-C6 印出來的文字 log，再用 regex 解析，例如：

```text
Bike speed = 12.34 KPH
Cadence = 80 RPM
Rider torque = 15.20 Nm
Motor RPM = 120 RPM
Assist mode = sport
Battery SOC = 98 %
Battery voltage = 47810 mV
Battery current = 70 mA
Battery temps = 25, 26, 25, 26 C
Rear gear = 4 / 12
```

它會維護一份目前最新 telemetry 狀態，並且每 100 ms 寫一筆快照到 CSV。

CSV header：

```csv
speed(kph),cadence(rpm),torque(nm),motor_speed(rpm),riding_mode,battery_capacity(%),battery_voltage(mv),battery_current(ma),battery_temp1(c),battery_temp2(c),battery_temp3(c),battery_temp4(c),rear_gear,time
```

輸出檔名會自動遞增：

```text
ccpa_telemetry_001.csv
ccpa_telemetry_002.csv
...
```

## 三路同步收資料的啟動流程

總啟動腳本：

```text
IMUtest/tools/record_three_esp32.sh
```

使用方式：

```bash
cd /home/chc/code/IMUtest

./tools/record_three_esp32.sh \
  --imu-a /dev/ttyUSB0 \
  --imu-b /dev/ttyUSB1 \
  --telemetry /dev/ttyACM0
```

可選參數：

```text
--baud BAUD       預設 115200
--outdir DIR      預設 csv_logs
--sdk-root DIR    預設 /home/chc/code/sdkTest
```

腳本內部流程：

1. 檢查 `--imu-a`、`--imu-b`、`--telemetry` 是否都有給。
2. 編譯 IMU 用的 host logger：

   ```bash
   g++ -std=c++17 IMUtest/tools/serial_csv_logger.cpp -o IMUtest/tools/serial_csv_logger
   ```

3. 編譯 CAN telemetry 用的 host logger：

   ```bash
   g++ -std=c++17 sdkTest/tools/serial_csv_logger.cpp -o sdkTest/tools/serial_csv_logger
   ```

4. 建立 run 資料夾，例如：

   ```text
   IMUtest/csv_logs/run_20260528_155603/
   ```

5. 建立三個子資料夾：

   ```text
   imu_a/
   imu_b/
   telemetry/
   ```

6. 寫入 `manifest.txt`，記錄本次使用的 port、baud、輸出位置、logger 路徑。
7. 背景啟動三個 logger：

   ```bash
   IMU logger A
   IMU logger B
   telemetry logger
   ```

8. 使用者按 `Ctrl+C` 後，腳本會停止三個 logger，並在 manifest 補上停止時間與 exit status。

## 一次 run 的輸出結構

實際會長得像：

```text
IMUtest/csv_logs/run_20260528_155603/
├── manifest.txt
├── imu_a/
│   └── imu_test_001.csv
├── imu_b/
│   └── imu_test_001.csv
└── telemetry/
    └── ccpa_telemetry_001.csv
```

`manifest.txt` 會記錄：

```text
run_id
started_at
baud
imu_a_port
imu_a_output_dir
imu_b_port
imu_b_output_dir
telemetry_port
telemetry_output_dir
imu_logger
telemetry_logger
sdk_root
stopped_at
exit_status
```

這很重要，因為之後分析資料時，可以知道每個 CSV 是從哪一個實體 serial port 來的。

## 啟動前檢查清單

### 1. 確認板子有插上

Linux 下可用：

```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

常見對應可能是：

```text
/dev/ttyUSB0  IMU A
/dev/ttyUSB1  IMU B
/dev/ttyACM0  ESP32-C6 CAN telemetry
```

實際 port 可能會因插入順序改變，所以每次開跑前最好確認。

### 2. 確認 baud rate

目前 firmware 與 logger 都以 115200 為主。

```text
Serial.begin(115200)
monitor_speed = 115200
record_three_esp32.sh default baud = 115200
```

### 3. 確認 firmware 是否燒對

IMU ESP32 應該燒 `IMUtest`。

```bash
cd /home/chc/code/IMUtest
pio run -t upload
```

CAN ESP32-C6 應該燒 `sdkTest`。

```bash
cd /home/chc/code/sdkTest
pio run -t upload
```

如果同時插很多板子，可能需要指定 upload port。

### 4. 確認 CAN 接線

CAN bus 不是普通 UART。CAN 端通常需要：

```text
CANH
CANL
GND reference
終端電阻，依 bus 架構決定
正確 bitrate，這裡目前是 250 kbps
```

ESP32-C6 firmware 目前支援兩種 CAN 來源：

```text
TWAI internal CAN controller
MCP2515 SPI CAN controller
```

實際硬體接哪一組，要看板子與轉接電路。

## 單獨收一顆 IMU

如果只想測一顆 IMU：

```bash
cd /home/chc/code/IMUtest
./tools/monitor_csv_logger.sh /dev/ttyUSB0 115200 csv_logs
```

或用 Python 版：

```bash
cd /home/chc/code/IMUtest
python3 tools/record_imu.py --port /dev/ttyUSB0 --baud 115200
```

C++ 版會輸出 `imu_test_001.csv` 這種檔名；Python 版會依時間產生 `bike_imu_YYYYMMDD_HHMMSS.csv`。

## 單獨收 CAN telemetry

如果只想測 CAN telemetry：

```bash
cd /home/chc/code/sdkTest
./tools/monitor_csv_logger.sh /dev/ttyACM0 115200 csv_logs
```

輸出會在：

```text
sdkTest/csv_logs/ccpa_telemetry_001.csv
```

## Dashboard 工具

目前有幾個輔助頁面：

```text
IMUtest/tools/imu_live_dashboard.html
IMUtest/tools/ccpa_telemetry_dashboard.html
sdkTest/tools/ccpa_telemetry_dashboard.html
sdkTest/tools/csv_telemetry_dashboard.html
```

用途大致是：

- `imu_live_dashboard.html`：用瀏覽器 Web Serial 看即時 IMU。
- `ccpa_telemetry_dashboard.html`：用瀏覽器看即時 CAN telemetry。
- `csv_telemetry_dashboard.html`：讀已經存好的 telemetry CSV 做摘要/預覽。

`sdkTest/tools/serial_web_dashboard.py` 會開一個簡單 web server，讓 dashboard 可以透過主機端 Python 幫忙串 serial。

## UART、USB Serial、CAN 的差異

這個專案裡有三種概念容易混在一起。

### UART / Serial

UART 是一種序列通訊。最基本線路是：

```text
TX
RX
GND
```

它通常是一對一通訊。ESP32 程式中的：

```cpp
Serial.begin(115200);
Serial.print(...);
```

就是在用 Serial/UART 的概念。當 ESP32 接到電腦 USB 後，Linux 會看到 `/dev/ttyUSB0` 或 `/dev/ttyACM0`。

### USB Serial

USB Serial 是電腦看到的形式。對電腦端 logger 來說，不管底層是真 UART-to-USB 晶片，還是 ESP32-C6 的 USB CDC，最後都像一個 serial port：

```text
/dev/ttyUSB0
/dev/ttyACM0
```

### CAN

CAN 是車用常見 bus，線路通常是：

```text
CANH
CANL
```

CAN frame 有 ID、DLC、data、仲裁、ACK、CRC 等機制。它不是單純把文字從 TX/RX 傳出去。

所以如果 KNEO Pi 沒有直接支援 CAN，就不能只靠一般 GPIO 直接接到 CANH/CANL。需要 CAN controller/transceiver，或使用現成轉接器。

## URAT

UART，意思是：

```text
KNEO Pi UART
   ↓
UART-to-CAN 轉接器
   ↓
CAN bus
```

如果 KNEO Pi 不能直接送 CAN frame，常見選項有：

```text
KNEO Pi --USB--> USB-to-CAN adapter --CANH/CANL--> CAN bus
KNEO Pi --UART--> UART-to-CAN module --CANH/CANL--> CAN bus
KNEO Pi --UART/USB Serial--> ESP32-C6 --CANH/CANL--> CAN bus
```

目前 `sdkTest` 比較像：

```text
CAN bus -> ESP32-C6 -> USB Serial -> 電腦端 CSV
```

如果未來要讓 KNEO Pi 主動發 CAN，`sdkTest` 可以被改成：

```text
KNEO Pi -> UART/USB Serial -> ESP32-C6 -> CAN bus
```

也就是在 ESP32-C6 firmware 裡新增一個 serial command parser，讓 KNEO Pi 傳入類似：

```text
CAN,1E942040,8,01,02,03,04,05,06,07,08
```

ESP32-C6 解析後呼叫：

```cpp
CAN_base_transmit(...)
```

或：

```cpp
canbus_spi_transmit(...)
```

把它真正送到 CAN bus。

## 目前程式的幾個重要特性

### 1. IMU 的時間有兩種

IMU CSV 裡有：

```text
host_time_s
t_ms
```

`t_ms` 是 ESP32 自己從開機後開始算的 `millis()`。

`host_time_s` 是電腦收到資料時的時間。之後如果要對齊兩顆 IMU 和 CAN telemetry，通常會用 host time 比較方便，但它會包含 USB/Serial 傳輸延遲。

### 2. CAN telemetry 是 100 ms 快照

CAN telemetry CSV 不是每收到一個 CAN frame 就寫一筆，而是每 100 ms 寫一筆「目前最新狀態」。

所以它比較像：

```text
每 100 ms 記錄一次車況狀態
```

不是：

```text
每一個 raw CAN frame 都存一行
```

如果未來要做非常精準的 CAN frame 分析，可能要另外加 raw CAN CSV logger。

### 3. `IMUtest` 其實也保留了 CAN 程式碼

`IMUtest/src` 裡也有 `CAN_base.cpp`、`s_canbus_spi.cpp`、`ccpa_telemetry.c` 等檔案。

但因為目前 `platformio.ini` 設定：

```ini
-DENABLE_CAN_BRIDGE=0
```

所以 `IMUtest` 現在編出來不跑 CAN，只跑 IMU。

### 4. `sdkTest` 沒有 IMU logger

`sdkTest/src` 裡沒有 `imu_logger.cpp`，所以它目前不是拿來讀 MPU6050 的。它的重點是 CAN/CCPA telemetry。

### 5. 三路資料目前不是硬體同步觸發

`record_three_esp32.sh` 會幾乎同時啟動三個 logger，但不是硬體層級的同步觸發。

也就是：

- 三個 CSV 在同一個 run 資料夾。
- 三個 logger 啟動時間很接近。
- 但三個感測來源沒有共用同一個硬體時鐘。

如果之後要做高精度同步，需要額外設計，例如：

```text
共用 trigger pin
共用 PPS/time sync
ESP32 收到同步命令後一起開始輸出
在資料中加入同步事件 marker
```

## 常見問題與排查

### 沒有產生 CSV

檢查：

```text
serial port 是否正確
baud 是否 115200
板子是否有燒對 firmware
使用者是否有權限讀 /dev/ttyUSB*
```

Linux 權限問題可先看：

```bash
ls -l /dev/ttyUSB0 /dev/ttyACM0
groups
```

通常使用者需要在 `dialout` 群組。

### IMU CSV 只有 header，沒有資料

可能原因：

```text
MPU6050 沒接好
SDA/SCL 接錯
I2C 位址不是 0x68/0x69
ESP32 firmware 沒有跑 IMUtest
Serial port 選到別的板子
```

IMU firmware 找不到 MPU6050 時會印：

```text
# MPU6050 not found on I2C address 0x68 or 0x69
```

### telemetry CSV 都是 0 或空值

可能原因：

```text
CANH/CANL 沒接到正確 bus
CAN bitrate 不符，目前是 250 kbps
MCP2515 或 TWAI 接線不對
車上沒有送對應的 CCPA frame
logger 有收到 serial，但 ESP32 沒解析到 telemetry
```

### `/dev/ttyUSB0` 和 `/dev/ttyUSB1` 對調

這很常見。USB 插入順序改變後，Linux 可能重新分配 port。

建議每次開跑前：

```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

必要時單獨開 serial monitor 看哪一顆是 IMU A/B。

## 建議的日常操作流程

1. 插上兩顆 IMU ESP32 和一顆 CAN ESP32-C6。
2. 確認 serial port：

   ```bash
   ls /dev/ttyUSB* /dev/ttyACM*
   ```

3. 確認 IMU A/B/telemetry 各自是哪個 port。
4. 從 `IMUtest` 啟動三路 logger：

   ```bash
   cd /home/chc/code/IMUtest

   ./tools/record_three_esp32.sh \
     --imu-a /dev/ttyUSB0 \
     --imu-b /dev/ttyUSB1 \
     --telemetry /dev/ttyACM0
   ```

5. 開始測試。
6. 測完按 `Ctrl+C`。
7. 到 `IMUtest/csv_logs/run_.../` 找資料。
8. 先看 `manifest.txt`，確認本次 port 對應正確。
9. 再分析：

   ```text
   imu_a/imu_test_001.csv
   imu_b/imu_test_001.csv
   telemetry/ccpa_telemetry_001.csv
   ```

## 後續可以改進的方向

### 1. 自動辨識 serial port

目前要手動輸入：

```text
--imu-a
--imu-b
--telemetry
```

之後可以用 USB VID/PID、serial number 或 udev rules 固定名字，例如：

```text
/dev/imu_a
/dev/imu_b
/dev/can_telemetry
```

這樣就不怕 `/dev/ttyUSB0` 和 `/dev/ttyUSB1` 對調。

### 2. 增加 raw CAN CSV

目前 telemetry CSV 是解析後的 100 ms 快照。若要 debug CAN protocol，可以新增 raw CAN logger：

```csv
host_time_s,bus,id,dlc,data0,data1,data2,data3,data4,data5,data6,data7
```

這樣日後可以回放或重新解析。

### 3. 增加同步 marker

可以讓三個 ESP32 都支援某個指令，例如收到：

```text
MARK,START
```

就各自在 Serial 輸出：

```text
MARK,START,t_ms
```

這樣 CSV 對齊會更容易。

### 4. 把 ESP32-C6 改成 UART-to-CAN bridge

如果 KNEO Pi 需要主動控制 CAN bus，可以在 `sdkTest` 新增 serial command parser，把 KNEO Pi 的 UART/USB Serial 指令轉成 CAN frame。

可能資料流：

```text
KNEO Pi
  ↓ UART/USB Serial command
ESP32-C6 sdkTest
  ↓ CAN frame
CAN bus
```

同時也可以保留目前的：

```text
CAN bus
  ↓
ESP32-C6 sdkTest
  ↓ USB Serial telemetry
CSV logger
```

讓同一顆 ESP32-C6 同時做：

```text
Serial -> CAN
CAN -> Serial telemetry
```

## 心智模型

最簡單的記法：

```text
IMUtest = 讀 IMU + IMU CSV logger + 三路總啟動腳本
sdkTest = 讀 CAN/CCPA + telemetry CSV logger

record_three_esp32.sh = 把兩個 IMU + 一個 CAN telemetry 綁成同一次實驗
```

目前資料不是三顆感測器直接互相溝通，而是三顆板子各自把資料送到主機，由主機端腳本把它們收在同一個 run 資料夾裡。
