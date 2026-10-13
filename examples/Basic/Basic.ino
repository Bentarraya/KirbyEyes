/*
 * KirbyEyes - Basic
 * ESP32-C3 + OLED SSD1306 128x64 I2C (alamat 0x3C)
 *
 * Awalnya ganti ekspresi otomatis tiap 4 detik (demo).
 * Buka Serial Monitor 115200 lalu tekan angka buat milih ekspresi manual:
 *   0 idle  1 excited  2 laughing  3 suspicious
 *   4 angry 5 confused 6 scared    7 sleeping
 *
 * Catatan PlatformIO: kalau Serial kosong di ESP32-C3, jangan pakai
 * -DARDUINO_USB_CDC_ON_BOOT=0 di board asli (flag itu cuma buat Wokwi).
 */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <KirbyEyes.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
KirbyEyes kirby(display);

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin();   // atau Wire.begin(SDA, SCL) kalau pin lu beda
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 ga ketemu, cek wiring/alamat I2C");
    for (;;) delay(1000);
  }

  kirby.begin();                  // 30 FPS
  kirby.setShakeTime(1500);       // getar nyala 1,5 detik tiap ganti ekspresi
  kirby.setAutoCycle(true, 4000); // demo: ganti ekspresi sendiri
}

void loop() {
  kirby.update();   // wajib, jangan pakai delay() di loop

  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '0' && c < '0' + KIRBY_BUILTIN_COUNT) {
      kirby.setAutoCycle(false);    // matiin demo, kendali manual
      kirby.setMood(c - '0');
    }
  }

  // Contoh di proyek asli (panggil tiap loop juga aman):
  //   if (jarak < 10)            kirby.setMood(MOOD_SCARED);
  //   else if (digitalRead(BTN)) kirby.setMood(MOOD_ANGRY);
  //   else                       kirby.setMood(MOOD_IDLE);
}
