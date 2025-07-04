# 🚀 雙電梯模擬器

這是一個使用 **C 語言** 製作的模擬系統，模擬兩部電梯同時接送乘客的過程，支援以下功能：

---

## ✅ 專案特色

- ✨ 雙電梯任務分配模擬（根據距離指派）
- 🔁 多執行緒模擬電梯同時運作（使用 `_beginthreadex`）
- 📈 動態樓層動畫顯示（`system("cls")` + `Sleep()`）
- 🧠 任務歷史紀錄保存（顯示每部電梯服務過哪些任務）
- 🛡 安全同步輸出（`CRITICAL_SECTION` 確保畫面正確）

---

## 🖼️ 執行畫面預覽（ASCII 動畫）

```
=== 電梯模擬器 ASCII 動畫 ===
樓層 10 |      
樓層  9 |      
樓層  8 |      
樓層  7 |      
樓層  6 |      
樓層  5 |      
樓層  4 |      
樓層  3 | [E1]
樓層  2 |      
樓層  1 |     [E2]
------------------------------
```

---

## 📋 範例輸入流程

```
== 雙電梯模擬器 + 任務紀錄（ASCII 動畫）==

輸入起點與終點樓層（例如 1 3）：1 5  
繼續輸入？1=是, 0=否：1  
輸入起點與終點樓層（例如 1 3）：8 2  
繼續輸入？1=是, 0=否：0
```

---

## 📊 模擬結束後任務紀錄輸出

```
模擬完成，共 2 筆請求。

電梯 1 任務紀錄：
  任務 1：從 1 樓 到 5 樓

電梯 2 任務紀錄：
  任務 1：從 8 樓 到 2 樓
```

---

## 📁 專案檔案結構

```
elevator_simulator.c    # 主程式
README.md               # 專案說明文件
screenshot.png          # 執行畫面截圖（可選）
```

---

## 🧠 延伸功能建議

- ✅ 加入 Gantt 圖時間軸輸出
- ✅ 加入方向優先（SCAN 排程邏輯）
- ✅ 模擬開門、關門時間
- ✅ 多人同方向上車邏輯
- ✅ 業界級智慧派梯邏輯（動態重新分配）
