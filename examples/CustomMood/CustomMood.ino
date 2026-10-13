/*
 * KirbyEyes - CustomMood
 * Nambah ekspresi sendiri dari sketch, tanpa ngubah library.
 *
 * Format pose:
 * {cx, cy, bodyR, bump, gap, lookX, lookY, bounce, shakeX, shakeY, glance,
 *  {w, h, ang, dx, dy},   <- mata KIRI
 *  {w, h, ang, dx, dy}}   <- mata KANAN
 *
 * Mata: w tebal, h panjang (h >= w), ang sudut (0 tegak, 90 rebah,
 * + "/" , - "\"), dx/dy geser mata itu saja.
 */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <KirbyEyes.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
KirbyEyes kirby(display);

// Wink: mata kiri melek, mata kanan jadi garis rebah
const KirbyPose POSE_WINK = {64, 33, 24, 9, 20, 0, 0, 0, 0, 0, 1,
                             {8, 18, 0, 0, 0}, {3, 12, 90, 0, 2}};

// Bored: mata pipih, badan agak turun, lirik ke bawah
const KirbyPose POSE_BORED = {64, 35, 24, 9, 22, 0, 4, 0, 0, 0, 0,
                              {10, 6, 90, 0, 0}, {10, 6, 90, 0, 0}};

int moodWink, moodBored;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) delay(1000);
  }

  kirby.begin();
  moodWink  = kirby.addMood(POSE_WINK);    // return nomor ekspresi (-1 kalau penuh)
  moodBored = kirby.addMood(POSE_BORED);

  // Ubah ekspresi bawaan juga bisa, misal angry lebih miring:
  KirbyPose* angry = kirby.pose(MOOD_ANGRY);
  if (angry) {
    angry->eyeL.ang = -30;
    angry->eyeR.ang = 30;
  }
}

uint32_t last = 0;
int step = 0;

void loop() {
  kirby.update();

  // contoh: gantian wink, bored, angry tiap 3 detik
  if (millis() - last > 3000) {
    last = millis();
    switch (step++ % 3) {
      case 0: kirby.setMood(moodWink);  break;
      case 1: kirby.setMood(moodBored); break;
      case 2: kirby.setMood(MOOD_ANGRY); break;
    }
  }
}
