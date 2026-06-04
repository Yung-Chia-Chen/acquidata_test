# sdkTest/tools/serial_csv_logger Buffered Collection

## 目的

記錄 `sdkTest/tools/serial_csv_logger.cpp` 的最新修改，確保 telemetry 直接從 raw CAN 封包解譯，並且：

1. 直接從 CCPA raw CAN packets 解析 telemetry
2. 將解析結果緩衝為可讀欄位
3. 每 0.5 秒建立一筆 telemetry snapshot
4. 在使用者按下 Ctrl+C 後一次寫出 CSV
5. 產生 Excel 友善、24 小時制的 `time` 欄位

## 變更內容

- `sdkTest/tools/serial_csv_logger.cpp`
  - 由 raw CAN line 解析 CCPA 封包，不再仰賴先前只用文字 log 的方案
  - 建立 `TelemetryRow::timestamp`，讓每次 0.5 秒取樣時即時記錄時間
  - 將 `time` 欄位格式改為 `YYYY/MM/DD HH:MM:SS`
  - 寫入 CSV 時，改為以純文字字串輸出 `time`，避免 Excel 自動轉成 AM/PM
  - 修改 `rear_gear` 為 `current(max)` 顯示，避免自動轉為日期
  - `battery_voltage(v)` 現在以 CCPA voltage raw 值轉換後的 Volt 顯示
  - `battery_current(ma)` 保留為原始 CCPA 毫安值

## CCPA 對應欄位

- `speed(kph)`
  - 來源：`GENERAL_INFO00ACK/BRO` 或 `CONTROLLER_INFO00ACK/BRO`
  - 解析：`u16_le` raw × 0.01

- `cadence(rpm)`
  - 來源：`GENERAL_INFO00ACK/BRO` byte 4，若不存在則 `CONTROLLER_INFO03ACK/BRO` byte 3

- `torque(nm)`
  - 來源：`GENERAL_INFO00ACK/BRO` bytes 2-3，scale 0.01
  - 備援：`CONTROLLER_INFO03ACK/BRO` bytes 4-5，scale 0.1

- `motor_speed(rpm)`
  - 來源：`CONTROLLER_INFO02ACK/BRO` bytes 4-5

- `riding_mode`
  - 來源：`Assist mode` / `Assist level` 文字解析
  - 以及 `CENTRAL_INTERFACE_ASSISTREQ` raw CAN assist level

- `battery_capacity(%)`
  - 來源：`GENERAL_INFO01ACK/BRO` byte 4
  - 備援：`BAT1_INFO01ACK/BRO` byte 6

- `battery_voltage(v)`
  - 來源：`BAT1_INFO01ACK/BRO` bytes 0-2
  - 解析：`u24_le` raw × 0.001

- `battery_current(ma)`
  - 來源：`BAT1_INFO01ACK/BRO` bytes 4-5
  - 單位：毫安，已依照 CCPA 協議直接呈現

- `battery_temp1..battery_temp4(c)`
  - 來源：`BAT1_INFO06ACK/BRO` bytes 0-3
  - 解析：raw − 64

- `rear_gear`
  - 來源：`REARDERAILLEUR_INFO00ACK/BRO` bytes 0,1
  - 顯示：`current(max)`

## 重要修正

1. 修正 `time` 欄位不是每筆都相同，改為每 0.5 秒取樣時立即記錄。
2. 修正 `battery_voltage(v)` 由 raw mV 值轉成 Volt 顯示。
3. 確認 `battery_current(ma)` 為 CCPA 原始毫安值，這是正確欄位。
4. 修正 `rear_gear` 顯示，避免 Excel 自動格式化。
5. 發現之前 `ccpa_telemetry_016.csv` 仍是舊版輸出，所以下一次要重新編譯並執行最新 `sdkTest/tools/serial_csv_logger`。

## 使用方式

1. 編譯最新 logger：
   ```bash
   cd /home/chc/code/sdkTest
   g++ -std=c++17 tools/serial_csv_logger.cpp -o tools/serial_csv_logger
   ```

2. 執行最新 logger：
   ```bash
   ./tools/serial_csv_logger /dev/ttyACM0 115200 csv_logs
   ```

3. 等待資料蒐集，按 `Ctrl+C` 停止後會寫出 CSV。

## 注意事項

- 若你打開 CSV 後仍看到 Excel 轉換成日期，請確認你使用的是最新生成的 CSV，而不是舊的 `IMUtest/csv_logs/run_*` 檔案。
- `time` 現在輸出為純文字格式，Excel 不應該再顯示 PM/AM。
- `battery_current(ma)` 是預期的 CCPA 毫安顯示，不需要再做額外換算。
