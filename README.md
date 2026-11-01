# KirbyEyes

Smooth animated Kirby character face for **SSD1306 128x64 (I2C, Adafruit)** OLED displays on ESP32.
Expressions are calculated with `lerp`, not frame-by-frame bitmaps. 11 built-in moods,
custom poses, blink/wink, gaze control and an inverted mode.

<img width="1208" height="686" alt="kirby_preview" src="https://github.com/user-attachments/assets/2a5c0309-e810-4a1c-87ce-f8eda0fd3a4b" />

## Installation

- **PlatformIO** edit your `platformio.ini`:

  ```ini
  lib_deps =
    adafruit/Adafruit SSD1306
    adafruit/Adafruit GFX Library
    https://github.com/Bentarraya/KirbyEyes.git
  ```
- **Arduino IDE**: clone or copy this repo into your `libraries/` directory
  (keep the folder name `KirbyEyes`).

## Wiring (ESP32-C3)

| OLED | ESP32-C3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO8 |
| SCL | GPIO9 |

Default I2C address is `0x3C`; change it in `display.begin()` if your module is `0x3D`.
If you use different pins, call `Wire.begin(SDA, SCL)` before `display.begin()`.

## Usage

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <KirbyEyes.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
KirbyEyes kirby(display);

void setup() {
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) delay(1000);
  }
  kirby.begin();              // 30 FPS
}

void loop() {
  kirby.update();             // required in every loop, non-blocking
  kirby.setMood(MOOD_ANGRY);  // safe to call every loop (ignored if same mood)
  // kirby.lookAt(100, 20);   // eyes follow a point on screen
  // kirby.wink();            // one-eye blink
}
```

## Built-in Moods

| ID | Mood |
|---|---|
| 0 | `MOOD_IDLE` |
| 1 | `MOOD_EXCITED` |
| 2 | `MOOD_LAUGHING` |
| 3 | `MOOD_SUSPICIOUS` |
| 4 | `MOOD_ANGRY` |
| 5 | `MOOD_CONFUSED` |
| 6 | `MOOD_SCARED` |
| 7 | `MOOD_SLEEPING` |
| 8 | `MOOD_WINK` |
| 9 | `MOOD_BORED` |
| 10 | `MOOD_SURPRISED` |

## API Reference

| Function | Description |
|---|---|
| `begin(fps=30)` | Call after `display.begin()`. |
| `update()` | Call in every loop. Returns `true` if one frame was drawn. |
| `setMood(m, restart=false)` | Change expression. `restart=true` forces a redraw (e.g., re-triggers shake animation). |
| `getMood()` / `moodCount()` | Get active expression / total number of expressions. |
| `addMood(pose)` | Add a custom expression, returns its ID (-1 if full). |
| `pose(m)` | Returns a pointer to the pose for runtime modification. |
| `setShakeTime(ms)` | Duration of shake animation after mood selection (0 = continuous). |
| `setEase(e)` | 0.1 for smooth, 0.4 for snappy transitions (time-corrected, same feel at any FPS). |
| `setFPS(f)` | Frames per second (1..60). |
| `setAutoBlink(b)` / `setAutoGlance(b)` | Enable/disable random blinking / random glancing. |
| `setAutoCycle(on, ms)` | Enable/disable automatic mood cycling (safe to call every loop, minimum interval 100 ms). |
| `setLook(x, y)` | Set the gaze offset in pixels. Persists across mood changes; reset with `setLook(0, 0)`. |
| `lookAt(x, y)` | Point both eyes at a screen coordinate; offset is clamped to 6 px. |
| `blink()` | Trigger a two-eye blink. |
| `wink(leftEye=true)` | Brief one-eye blink (default: screen-left eye). |
| `setInverted(on)` | Swap colors: white background, black body. |

## Pose Format

```
// eyeL (screen left) | eyeR (screen right)
// MOOD {cx, cy, bodyR, bump, gap, lookX, lookY, bounce, shakeX, shakeY, glance,}   {w, h, ang, dx, dy} | {w, h, ang, dx, dy}
```

| Parameter | Description |
|---|---|
| `cx, cy` | Body center position (`cy` larger = lower). |
| `bodyR` / `bump` | Body radius / hand radius. |
| `gap` | Distance between eye centers. |
| `lookX, lookY` | Gaze direction for both eyes. |
| `bounce` | Slow up-and-down movement. |
| `shakeX` | Fast horizontal shake (e.g., angry). |
| `shakeY` | Fast vertical shake (e.g., scared/laughing). |
| `glance` | 1 = allow random glances, 0 = static eyes. |
| `w, h` | Width and height of the eye capsule (`h >= w`). |
| `ang` | Angle: 0 for vertical, 90 for horizontal, + for `/` tilt, - for `\` tilt. |
| `dx, dy` | Offset for that specific eye. |

`eyeL` is drawn on the left side of the screen, `eyeR` on the right.

## Notes

- `update()` is non-blocking; do not call `delay()` in `loop()`.
- Up to 20 expressions total: 11 built-in + 9 custom (`KIRBY_MAX_MOODS`). Static RAM is ~1.9 KB.
- `MOOD_SLEEPING` disables automatic blinking; call `setMood()` to wake up.
- `lookAt()` is easier to keep steady with `setAutoGlance(false)`.
- Adafruit's SSD1306 driver switches I2C to 400 kHz during transfers, so 30 FPS is realistic on ESP32-C3.

## Examples

- `examples/Basic` — serial mood selector plus `blink` / `wink` / invert demos.
- `examples/CustomMood` — add custom poses and tweak built-in poses at runtime.
