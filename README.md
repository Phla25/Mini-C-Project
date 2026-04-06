# Hệ Thống Giám Sát Môi Trường

Chương trình mô phỏng hệ thống thu thập và xử lý dữ liệu từ các cảm biến môi trường, viết bằng C thuần. Hệ thống đọc cấu hình cảm biến và dữ liệu đầu vào từ file CSV, xử lý lọc nhiễu, phát hiện lỗi, ghi log sự kiện và xuất báo cáo tổng hợp.

---

## Cấu Trúc Thư Mục

```
project/
├── Files/
│   ├── Data.csv            # File dữ liệu đầu vào
│   ├── Sensors.csv         # File cấu hình danh sách cảm biến
├── Inc/
│   ├── ErrorHandler.h      # Định nghĩa mã lỗi và struct SystemReport
│   ├── DataHandler.h       # Struct DataNode, DataList (lịch sử dữ liệu)
│   ├── SensorHandler.h     # Struct Sensor, SensorList và các hàm xử lý
│   ├── LoggingHandler.h    # Ghi log sự kiện và xuất báo cáo
│   └── CSVHandler.h        # Đọc dữ liệu đầu vào từ file CSV
├── Src/
│   ├── main.c              # Điểm khởi chạy, hàm processIncomingData
│   ├── ErrorHandler.c
│   ├── DataHandler.c
│   ├── SensorHandler.c
│   ├── LoggingHandler.c
│   └── CSVHandler.c
├── system_events.log       # Log sự kiện (tự tạo khi chạy)
├── BaoCao_TongHop.txt      # Báo cáo tổng hợp (tự tạo khi chạy)
└── README.md
```

---

## Yêu Cầu

- GCC (MinGW hoặc MSYS2 trên Windows)
- Không cần thư viện ngoài, chỉ dùng C standard library

Kiểm tra GCC đã cài chưa:
```bash
gcc --version
```

---

## Compile và Chạy

Mở terminal trong thư mục `project/`, chạy lần lượt:

**Bước 1 — Compile:**
```bash
gcc Src/main.c Src/ErrorHandler.c Src/DataHandler.c Src/SensorHandler.c Src/LoggingHandler.c Src/CSVHandler.c -o program -I Inc/
```

**Bước 2 — Chạy với file CSV mặc định** (`Sensors.csv` và `Data.csv`):
```bash
./program
```

**Hoặc chỉ định file CSV tùy chỉnh:**
```bash
./program duong/dan/Sensors.csv duong/dan/Data.csv
```

---

## Định Dạng File CSV Đầu Vào

### `Sensors.csv` — Cấu hình cảm biến

```
id,type,longitude,latitude,min_threshold,max_threshold,sending_cycle,buffer_size
1,TEMP,21028,105834,15000,35000,1000,100
2,HUMIDITY,21030,105836,30000,80000,2000,50
3,LIGHT,21025,105830,0,90000,500,200
```

| Trường | Kiểu | Mô tả |
|---|---|---|
| `id` | uint32 | ID duy nhất của cảm biến |
| `type` | string | Loại cảm biến: `TEMP`, `HUMIDITY`, `LIGHT`, `GAS`, `DUST` |
| `longitude` | int32 | Kinh độ (fixed-point ×1000) |
| `latitude` | int32 | Vĩ độ (fixed-point ×1000) |
| `min_threshold` | int32 | Ngưỡng cảnh báo dưới (fixed-point ×1000) |
| `max_threshold` | int32 | Ngưỡng cảnh báo trên (fixed-point ×1000) |
| `sending_cycle` | uint32 | Chu kỳ gửi dữ liệu (milliseconds) |
| `buffer_size` | uint32 | Số bản tin tối đa lưu trong bộ nhớ |

### `Data.csv` — Dữ liệu đầu vào

```
sensor_id,timestamp,raw_value
1,100001,25000
1,100002,40000
```

| Trường | Kiểu | Mô tả |
|---|---|---|
| `sensor_id` | uint32 | ID cảm biến gửi bản tin |
| `timestamp` | uint64 | Thời điểm gửi (milliseconds, phải tăng dần) |
| `raw_value` | int32 | Giá trị thô (fixed-point ×1000, ví dụ: 25000 = 25.000°C) |

---

## Biểu Diễn Số Thực (Fixed-Point ×1000)

Chương trình không dùng `float` hay `double`. Mọi giá trị đều được nhân 1000 trước khi lưu:

```
25.5°C   →  raw_value =  25500
-3.2°C   →  raw_value =  -3200
68.0%    →  raw_value =  68000
```

Giới hạn vật lý được kiểm tra theo từng loại cảm biến:

| Loại | Giới hạn dưới | Giới hạn trên |
|---|---|---|
| TEMP | -60.000°C | +85.000°C |
| HUMIDITY | 0% | 100% |
| LIGHT | 0 | — |
| DUST | 0 | — |
| GAS | 0% | 100% |

---

## Luồng Xử Lý Dữ Liệu

Mỗi bản tin đầu vào đi qua các bước sau trong `processIncomingData()`:

```
Bản tin đến
    │
    ├─► Tìm sensor theo ID          ── Không tìm thấy → ERR log, bỏ qua
    │
    ├─► Kiểm tra buffer đầy         ── Đầy → WARNING log, ghi đè bản tin cũ
    │
    ├─► validateSensorData()
    │       ├─ Timestamp lùi về quá khứ  → ERR_INVALID_TIMESTAMP
    │       ├─ Vượt giới hạn vật lý      → ERR_PHYSICAL_BOUNDS
    │       ├─ Vượt ngưỡng cảnh báo      → STATUS_WARNING_HIGH/LOW
    │       └─ Bình thường               → STATUS_VALID_NORMAL
    │
    ├─► filterSensorData()          ── Nhiễu gai (nhảy > 10.0 đơn vị) → NOISE log, bỏ qua
    │       └─ Moving average (cửa sổ 5 mẫu)
    │
    └─► updateSensorData()          ── Lưu vào DataList, cập nhật thống kê
```

---

## Mã Lỗi (ValidationStatus)

| Mã | Ý nghĩa | Hành động |
|---|---|---|
| `STATUS_VALID_NORMAL` | Dữ liệu hợp lệ, trong ngưỡng | Lưu vào history |
| `STATUS_WARNING_HIGH` | Vượt ngưỡng trên | Ghi log WARNING, vẫn lưu |
| `STATUS_WARNING_LOW` | Dưới ngưỡng dưới | Ghi log WARNING, vẫn lưu |
| `ERR_SENSOR_NOT_FOUND` | Không tìm thấy ID | Ghi log ERROR, bỏ qua |
| `ERR_INVALID_TIMESTAMP` | Timestamp không hợp lệ | Ghi log ERROR, bỏ qua |
| `ERR_PHYSICAL_BOUNDS` | Giá trị vô lý về vật lý | Ghi log ERROR, bỏ qua |

---

## File Output

### `system_events.log`
Ghi nối tiếp (append) mỗi sự kiện bất thường theo định dạng:
```
[TS: 100002] [WARNING] [Sensor 1] - Gia tri vuot nguong canh bao an toan!
[TS: 100003] [ERROR]   [Sensor 1] - Du lieu vo ly hoac sai lech thoi gian. Da bo qua.
[TS: 100004] [NOISE]   [Sensor 1] - Phat hien nhieu gai dot bien. Da loc bo.
```
File bị xóa và tạo lại mỗi lần khởi động chương trình.

### `BaoCao_TongHop.txt`
Báo cáo tổng hợp cuối phiên, bao gồm thống kê từng cảm biến: số bản tin hợp lệ/lỗi, giá trị lớn nhất/nhỏ nhất/trung bình.

---

## Thống Kê Hệ Thống (SystemReport)

In ra màn hình sau khi xử lý xong toàn bộ dữ liệu:

```
=== THONG KE HE THONG ===
Hop le         : 5      ← Bản tin hợp lệ (bao gồm cả WARNING)
Loi            : 4      ← Bản tin bị loại (timestamp sai, vật lý vô lý, ID không tồn tại, nhiễu)
Vuot nguong    : 1      ← Số lần kích hoạt cảnh báo ngưỡng
Buffer overflow: 0      ← Số lần buffer của sensor bị đầy
Packet dropped : 0      ← Số bản tin bị ghi đè do buffer đầy
```