# KirbyEyes

Smooth animated Kirby character face for **SSD1306 128x64 (I2C, Adafruit)** OLED displays, on ESP32-C3.
Expressions are calculated using `lerp`, not frame-by-frame bitmaps.

<img width="1208" height="686" alt="kirby_preview" src="https://github.com/user-attachments/assets/2a5c0309-e810-4a1c-87ce-f8eda0fd3a4b" />

## Installation

- **PlatformIO** Edit your 'platformio.ini' into:

  ```ini
  lib_deps =
    adafruit/Adafruit SSD1306
    adafruit/Adafruit GFX Library
    https://github.com/Bentarraya/KirbyEyes.git
  ```
- **Arduino IDE**: Clone or Place this repo in your `libraries/` directory.

## Usage

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
  kirby.update();             // required in every loop, non-blocking
  kirby.setMood(MOOD_ANGRY);  // safe to call every loop (ignored if same mood)
}
```

## Default Expressions

`MOOD_IDLE`, `MOOD_EXCITED`, `MOOD_LAUGHING`, `MOOD_SUSPICIOUS`, `MOOD_ANGRY`,
`MOOD_CONFUSED`, `MOOD_SCARED`, `MOOD_SLEEPING`

## API Reference

| Function | Description |
|---|---|
| `begin(fps=30)` | Call after `display.begin()` |
| `update()` | Call in every loop. Returns `true` if one frame was drawn. |
| `setMood(m, restart=false)` | Change expression. `restart=true` forces a redraw (e.g., re-triggers shake animation). |
| `getMood()` / `moodCount()` | Get active expression / total number of expressions. |
| `addMood(pose)` | Add a custom expression, returns its ID (-1 if full). |
| `pose(m)` | Returns a pointer to the pose for runtime modification. |
| `setShakeTime(ms)` | Duration of shake animation after mood selection (0 = continuous). |
| `setEase(e)` | 0.1 for smooth, 0.4 for snappy transitions. |
| `setFPS(f)` | Frames per second (1..60). |
| `setAutoBlink(b)` / `setAutoGlance(b)` | Enable/disable random blinking / random glancing. |
| `setAutoCycle(on, ms)` | Enable/disable automatic mood cycling for demo purposes. |
| `setLook(x, y)` | Manually set the direction of gaze. |
| `blink()` | Triggers a single blink.

## Pose Format

```
//											      Right Eye | Left Eye
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
| `dx, dy` | Offset for that specific eye.

Full examples: `examples/Basic` and `examples/CustomMood`.

