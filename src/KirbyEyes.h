/*
 * KirbyEyes - wajah Kirby animasi smooth buat OLED SSD1306 128x64 (Adafruit)
 *
 * Konsep: tiap ekspresi = satu "Pose" (kumpulan angka). Tiap frame, nilai
 * sekarang ngejar nilai target (lerp), jadi transisi mulus tanpa bitmap.
 *
 * Pemakaian minimal:
 *
 *   Adafruit_SSD1306 display(128, 64, &Wire, -1);
 *   KirbyEyes kirby(display);
 *
 *   setup(): Wire.begin(); display.begin(SSD1306_SWITCHCAPVCC, 0x3C); kirby.begin();
 *   loop():  kirby.update();            // wajib tiap loop, non-blocking
 *            kirby.setMood(MOOD_ANGRY); // ganti ekspresi kapan aja
 */
#ifndef KIRBY_EYES_H
#define KIRBY_EYES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Maksimal ekspresi (bawaan + custom dari addMood)
#define KIRBY_MAX_MOODS 20

// Satu mata = kapsul (bentuk pil)
//   w   ketebalan      h   panjang (harus >= w)
//   ang sudut putar derajat: 0 tegak, 90 rebah, + miring "/", - miring "\"
//   dx,dy geser posisi mata ini saja
struct KirbyEye {
  float w, h, ang, dx, dy;
};

// Satu ekspresi. SEMUA field harus float (di-lerp sebagai array float).
//   cx, cy   posisi tengah badan        bodyR  radius badan
//   bump     radius tangan              gap    jarak antar pusat mata
//   lookX/Y  arah pandang kedua mata    bounce goyang naik-turun pelan
//   shakeX   getar kiri-kanan cepat     shakeY getar atas-bawah cepat
//   glance   1 = boleh lirik acak, 0 = mata diam
struct KirbyPose {
  float cx, cy, bodyR, bump, gap, lookX, lookY, bounce, shakeX, shakeY, glance;
  KirbyEye eyeL, eyeR;
};

// Ekspresi bawaan (urutan = nomor ekspresi)
enum KirbyMood : uint8_t {
  MOOD_IDLE = 0,
  MOOD_EXCITED,
  MOOD_LAUGHING,
  MOOD_SUSPICIOUS,
  MOOD_ANGRY,
  MOOD_CONFUSED,
  MOOD_SCARED,
  MOOD_SLEEPING,
  MOOD_WINK,
  MOOD_BORED,
  MOOD_SURPRISED,
  KIRBY_BUILTIN_COUNT
};

class KirbyEyes {
 public:
  explicit KirbyEyes(Adafruit_SSD1306& display);

  // Panggil SETELAH display.begin(). Library ga nyentuh pin/alamat/clock I2C lu.
  void begin(uint8_t fps = 30);

  // Panggil tiap loop(). Return true kalau barusan nggambar 1 frame.
  bool update();

  // ---- ekspresi ----
  // Ganti ekspresi. Kalau ekspresinya sama dengan yang aktif, diabaikan
  // (aman dipanggil tiap loop). restart=true buat maksa ngulang (shake nyala lagi).
  void setMood(uint8_t mood, bool restart = false);
  uint8_t getMood() const { return mood_; }
  uint8_t moodCount() const { return moodCount_; }

  // Tambah ekspresi custom. Return nomor ekspresinya, atau -1 kalau penuh.
  int addMood(const KirbyPose& pose);
  // Akses pose (bawaan/custom) buat diubah runtime. nullptr kalau nomor ga valid.
  KirbyPose* pose(uint8_t mood);

  // ---- pengaturan ----
  void setShakeTime(uint16_t ms);   // lama getar sejak ekspresi dipilih (0 = terus-terusan)
  void setEase(float ease);         // 0.1 licin/lambat ... 0.4 snappy (default 0.22, dikoreksi per FPS)
  void setFPS(uint8_t fps);         // 1..60 (default 30)
  void setAutoBlink(bool on);       // kedip acak (default on)
  void setAutoGlance(bool on);      // lirik acak (default on)
  void setAutoCycle(bool on, uint16_t intervalMs = 5000);  // demo: ganti ekspresi otomatis
  void setLook(float x, float y);   // offset pandang manual (px), persist sampai di-reset
  void lookAt(float x, float y);    // arahin mata ke titik (x, y) di layar, offset maks 6 px
  void blink();                     // kedip dua mata sekarang
  void wink(bool leftEye = true);   // kedip satu mata (default mata kiri layar)
  void setInverted(bool on);        // background putih, badan hitam

 private:
  void behave(uint32_t now);
  void startBlink(bool left, bool right, uint16_t ms, uint32_t now);
  void draw(uint32_t now);
  void drawEye(float x, float y, const KirbyEye& e, float k,
               uint16_t eyeColor, uint16_t lidColor);

  Adafruit_SSD1306& display_;

  KirbyPose poses_[KIRBY_MAX_MOODS];
  uint8_t moodCount_ = KIRBY_BUILTIN_COUNT;
  uint8_t mood_ = MOOD_IDLE;

  KirbyPose cur_;
  KirbyPose target_;

  float ease_ = 0.22f;
  uint16_t frameMs_ = 33;
  uint16_t shakeMs_ = 1500;

  bool autoBlink_ = true;
  bool autoGlance_ = true;
  bool autoCycle_ = false;
  bool inverted_ = false;
  uint16_t cycleMs_ = 5000;

  float blinkL_ = 1.0f, blinkR_ = 1.0f;   // 1 = melek, 0 = tutup
  float blinkTargetL_ = 1.0f, blinkTargetR_ = 1.0f;
  uint32_t nextBlink_ = 0, blinkReopenL_ = 0, blinkReopenR_ = 0;
  float glanceX_ = 0, glanceY_ = 0;
  uint32_t nextGlance_ = 0;
  float lookOffX_ = 0, lookOffY_ = 0;

  uint32_t lastFrame_ = 0, lastCycle_ = 0, modeStart_ = 0;
};

#endif
