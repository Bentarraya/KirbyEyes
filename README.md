# KirbyEyes

Wajah karakter Kirby animasi smooth buat OLED **SSD1306 128x64 (I2C, Adafruit)**, ESP32-C3.
Ekspresi dihitung pakai lerp, bukan bitmap frame-by-frame.

## Install

- **PlatformIO**: taruh folder ini di `lib/KirbyEyes/` dan pastikan `platformio.ini` punya
  ```ini
  lib_deps =
    adafruit/Adafruit SSD1306
    adafruit/Adafruit GFX Library
  ```
- **Arduino IDE**: taruh di folder `libraries/`.

## Pakai

```cpp
#include <KirbyEyes.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
KirbyEyes kirby(display);

void setup() {
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  kirby.begin();
}

void loop() {
  kirby.update();             // wajib tiap loop, non-blocking
  kirby.setMood(MOOD_ANGRY);  // aman dipanggil tiap loop (sama = diabaikan)
}
```

## Ekspresi bawaan

`MOOD_IDLE`, `MOOD_EXCITED`, `MOOD_LAUGHING`, `MOOD_SUSPICIOUS`, `MOOD_ANGRY`,
`MOOD_CONFUSED`, `MOOD_SCARED`, `MOOD_SLEEPING`

## API

| Fungsi | Gunanya |
|---|---|
| `begin(fps=30)` | Panggil setelah `display.begin()` |
| `update()` | Tiap loop. Return `true` kalau nggambar 1 frame |
| `setMood(m, restart=false)` | Ganti ekspresi. `restart=true` maksa ulang (shake nyala lagi) |
| `getMood()` / `moodCount()` | Ekspresi aktif / jumlah ekspresi |
| `addMood(pose)` | Tambah ekspresi custom, return nomornya (-1 kalau penuh) |
| `pose(m)` | Pointer ke pose buat diubah runtime |
| `setShakeTime(ms)` | Lama getar sejak ekspresi dipilih (0 = terus) |
| `setEase(e)` | 0.1 licin, 0.4 snappy |
| `setFPS(f)` | 1..60 |
| `setAutoBlink(b)` / `setAutoGlance(b)` | Kedip / lirik acak |
| `setAutoCycle(on, ms)` | Demo ganti ekspresi otomatis |
| `setLook(x, y)` | Geser arah pandang manual |
| `blink()` | Kedip sekali |

## Format pose

```
{cx, cy, bodyR, bump, gap, lookX, lookY, bounce, shakeX, shakeY, glance,
 {w, h, ang, dx, dy},   // mata kiri
 {w, h, ang, dx, dy}}   // mata kanan
```

| Angka | Fungsi |
|---|---|
| `cx, cy` | posisi tengah badan (`cy` besar = turun) |
| `bodyR` / `bump` | radius badan / radius tangan |
| `gap` | jarak antar pusat mata |
| `lookX, lookY` | arah pandang kedua mata |
| `bounce` | goyang naik-turun pelan |
| `shakeX` | getar kiri-kanan cepat (marah) |
| `shakeY` | getar atas-bawah cepat (takut/ketawa) |
| `glance` | 1 = boleh lirik acak, 0 = mata diam |
| `w, h` | tebal dan panjang kapsul mata (`h >= w`) |
| `ang` | sudut: 0 tegak, 90 rebah, + miring `/`, - miring `\` |
| `dx, dy` | geser mata itu saja |

Contoh lengkap: `examples/Basic` dan `examples/CustomMood`.
