# GPIO Data Control 使用說明

此文件說明如何使用 `IMUtest/gpio_data_control.py`，透過 GPIO 按鈕控制三個資料蒐集程式。

## 硬體接線

- 按鈕：連接到 `GPIO27`
- LED：連接到 `GPIO23`（BCM 23，實體腳位 16），另一端接 GND（實體腳位 20）

按鈕採用內部上拉，按下時會觸發低電位。

## 功能

程式預設使用 **GPIO23** 控制單顆 LED 狀態：

- 按一次按鈕開始收集：
   - **LED 亮起**（GPIO23 high）
  - 啟動 `IMUtest/tools/record_three_esp32.sh`，開始收集三路串口資料
  - 數據蒐集開始時會亮起，表示系統正在進行數據收集

- 再按一次按鈕停止收集：
   - **LED 熄滅**（GPIO23 low）
  - 送出 `SIGINT` 停止三個資料蒐集程式
   - 停止狀態 LED 關閉，表示系統就緒

如果你有兩個 LED，可以使用 `--led-red-pin` 和 `--led-blue-pin` 來切換紅/藍顯示。

## 使用步驟 terminal

1. 進入專案資料夾：
   ```bash
   cd /home/chc/code/IMUtest
   ```
2. 開啟tmux
   ```
   tmux new -s imu
   ```
3. 執行控制程式：
   ```bash
   ./gpio_data_control.py
   ```

4. 按下按鈕開始收集，按第二次按鈕停止收集。

## tmux 常用指令

- 開新 session：
   ```bash
   tmux new -s imu
   ```
- 暫時離開 tmux（保持背景跑）：
   - 按 `Ctrl-b`，放開後按 `d`
- 回到既有 session：
   ```bash
   tmux attach -t imu
   ```
- 列出所有 session：
   ```bash
   tmux ls
   ```
- 結束指定 session：
   ```bash
   tmux kill-session -t imu
   ```

## SSH 斷線仍可收資料

只要 `gpio_data_control.py` 在 tmux 裡執行，SSH 斷線後仍會繼續跑。建議流程：

1. 進 tmux 執行程式
2. 斷線前按 `Ctrl-b` 然後 `d`
3. 需要看狀態時再用 `tmux attach -t imu`

## 樹莓派關機

```bash
sudo shutdown -h now
```

## 參數

如果要指定不同的串口或輸出目錄，可使用以下參數：

```bash
./gpio_data_control.py --imu-a /dev/ttyUSB0 --imu-b /dev/ttyUSB1 --telemetry /dev/ttyACM0 --baud 115200 --outdir csv_logs
```

如果你有兩個 LED，需要同時控制紅、藍兩個 GPIO，可以改用 `--led-red-pin` 和 `--led-blue-pin`：

```bash
./gpio_data_control.py --led-red-pin 23 --led-blue-pin 25
```

按下按鈕開始收數據時會切換成紅燈（GPIO23 on、GPIO25 off），停止收數據時會切換成藍燈（GPIO23 off、GPIO25 on）。

參數說明：

- `--imu-a`：IMU A 的序列埠
- `--imu-b`：IMU B 的序列埠
- `--telemetry`：Telemetry 的序列埠
- `--baud`：三個序列埠共用的 Baud rate
- `--outdir`：資料輸出根目錄
- `--led-pin`：紅 LED 的 BCM GPIO 編號（等同 `--led-red-pin`）
- `--led-red-pin`：紅 LED 的 BCM GPIO 編號
- `--led-blue-pin`：藍 LED 的 BCM GPIO 編號
- `--led-active-low`：如果 LED 接到 3.3V，使用 active-low 方式點亮

## LED 測試

在實際使用前，建議先測試 LED 和按鈕是否正常工作：

1. 基本 LED 測試（檢查 GPIO 輸出是否正確）：
    - 單顆 LED：
       ```bash
       python3 gpio_data_control.py --test-led
       ```
       會點亮 2 秒再關閉。

    - 兩顆 LED：
       ```bash
       python3 gpio_data_control.py --test-led --led-red-pin 23 --led-blue-pin 25
       ```
       會先顯示藍燈，再點亮紅燈 2 秒，最後回到藍燈。

2. 如果 LED 未亮，檢查清單：
   - 確認 GPIO23（實體 16）已正確接線
   - LED 正極接到 GPIO，負極接 GND（或確認極性接法）
   - 確認有串聯 220~330Ω 限流電阻
   - 如果 LED 是接到 3.3V 另一端接 GPIO，則加 `--led-active-low`

3. 按鈕測試（搭配數據收集）：
   ```bash
   python3 gpio_data_control.py
   ```
   - 程式啟動時，LED 應該是關閉（停止狀態）
   - 按下按鈕一次，應看見 LED 亮起，同時終端輸出開始收集訊息
   - 再按一次按鈕，應看見 LED 熄滅，同時終端輸出停止訊息

## 先決條件

- 安裝 `RPi.GPIO`：
  ```bash
  sudo apt install python3-rpi.gpio
  ```

- `IMUtest/tools/record_three_esp32.sh` 必須存在且可執行

## 注意事項

- 本程式會在背景啟動 `record_three_esp32.sh`，並監控該程序狀態。
- 如果按鈕按下後 LED 未亮，請確認 GPIO 配線與按鈕設定是否正確。
- 如果要終止腳本本身，請按 `Ctrl+C`。

## 斷網測試

此程式只依賴本機 GPIO，因此即使沒有網路也可以透過按鈕啟動與停止資料收集。

1. 確保 Raspberry Pi 已經連好按鈕、LED 與 IMU/Telemetry 裝置。
2. 進入專案資料夾：
   ```bash
   cd /home/chc/code/IMUtest
   ```
3. 直接啟動控制程式：
   ```bash
   python3 gpio_data_control.py
   ```
4. 按下按鈕一次，確認 LED 亮並且 `record_three_esp32.sh` 開始建立 `csv_logs` 資料。
5. 再按一次按鈕，確認資料收集程序停止、LED 熄滅。

如果你想先檢查 LED 與 GPIO 輸出是否正常，可先用測試模式：
```bash
python3 gpio_data_control.py --test-led
```

如果 LED 正極直接接到 `GPIO23`，應該使用預設的 active-high。若 LED 接到 `3.3V` 則改用 `--led-active-low`。

## 開機自動啟動（可選）

若希望開機後自動啟動 GPIO 監控程式，建議建立 systemd 服務檔案。

1. 建立 `/etc/systemd/system/gpio_data_control.service`：
   ```ini
   [Unit]
   Description=GPIO data control for IMU collection
   After=network.target

   [Service]
   Type=simple
   WorkingDirectory=/home/chc/code/IMUtest
   ExecStart=/usr/bin/python3 /home/chc/code/IMUtest/gpio_data_control.py
   Restart=always
   User=chc

   [Install]
   WantedBy=multi-user.target
   ```
2. 啟用並啟動：
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable gpio_data_control.service
   sudo systemctl start gpio_data_control.service
   ```

這樣 Raspberry Pi 開機後就會自動監控按鈕，斷網時仍可用按鈕觸發資料收集。
