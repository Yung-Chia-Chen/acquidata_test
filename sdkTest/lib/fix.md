1. src/ccpa_telemetry.c — Rear Derailleur 讀取邏輯


- if (frame->dlc >= 1 && state->rearGearSource != CCPA_TELEMETRY_SOURCE_GENERAL_INFO) {
+ if (frame->dlc >= 2) {
      state->rearGearIndex = frame->data[0];
+     state->rearGearMax = frame->data[1];   ← 新增讀取 GearRange（最大檔位數）
原始程式有一個 rearGearSource != GENERAL_INFO 的判斷，當 General Info 有優先權時就封鎖 Rear Derailleur 的更新。但 General Info 的 byte[6] 實測永遠是 0x00，導致每 100ms 就清掉資料，造成不穩定。移除這個條件後，Rear Derailleur INFO00 每次收到都能正常更新。

2. src/ccpa_telemetry.h — 新增欄位


+ uint8_t rearGearMax;   ← 新增儲存最大檔位數
3. src/main.cpp — Log 格式與接收 timeout


- MAIN_LOG_I("Rear gear index = %u", telemetryState.rearGearIndex);
+ MAIN_LOG_I("Rear gear = %u / %u", telemetryState.rearGearIndex, telemetryState.rearGearMax);

- canbus_spi_receive(&CAN_FrameRx, 50)   ← timeout 50ms
+ canbus_spi_receive(&CAN_FrameRx, 5)    ← 改為 5ms，減少延遲

- CAN_base_receive(&twaiFrameRx, 50)
+ CAN_base_receive(&twaiFrameRx, 5)
根本原因總結： 原本程式試圖從 General Info byte[6] 取絕對檔位，但實測該欄位始終為 0x00（系統未填值），且每 100ms 就覆蓋一次 Rear Derailleur 的資料。修正後改為直接從 Rear Derailleur INFO00 同時讀取 GearIndex（byte[0]）和 GearRange（byte[1]），顯示為「N / Max」格式。