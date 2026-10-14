#include "KirbyEyes.h"

// Pose di-lerp lewat array float, jadi struct-nya harus murni float tanpa padding.
static_assert(sizeof(KirbyPose) == 21 * sizeof(float),
              "KirbyPose harus murni float (11 + 2x5), jangan nambah tipe lain");

static const int POSE_FLOATS = sizeof(KirbyPose) / sizeof(float);

// Ekspresi bawaan, persis dari main.cpp.
//												                                                                     Right Eye , Left Eye
// MOOD {cx, cy, bodyR, bump, gap, lookX, lookY, bounce, shakeX, shakeY, glance,}		 {w, h, ang, dx, dy} , {w, h, ang, dx, dy} 	
static const KirbyPose BUILTIN_POSES[KIRBY_BUILTIN_COUNT] = {
  /* IDLE       */ {64, 33, 24,  9, 20, 0, 0, 0, 0,   0,   1,   {8, 18,   0, 0,  0}, {8, 18,   0,  0,  0}},
  /* EXCITED    */ {64, 33, 26, 11, 20, 2, 0, 3, 0,   0,   1,   {9, 24,   0, 0,  0}, {9, 24,   0,  0,  0}},
  /* LAUGHING   */ {64, 35, 23,  8, 18, 0, 0, 0, 0,   1.8, 0,   {8, 22,  12, 0,  0}, {8, 22, -12,  0,  0}},
  /* SUSPICIOUS */ {64, 33, 24,  9, 20, 0, 0, 0, 0,   0,   0.5, {6, 12,  94, 7,  1}, {6, 17,   4,  0, -1}},
  /* ANGRY      */ {64, 34, 24,  9, 20, 0, 0, 0, 1.6, 0,   0,   {8, 18, -22, 0,  1}, {8, 18,  22,  0,  1}},
  /* CONFUSED   */ {64, 33, 24,  9, 20, 0, 0, 0, 0,   0,   0.5, {6, 17,   4, 0, -1}, {6, 12,  94, -7,  1}},
  /* SCARED     */ {64, 35, 23,  8, 15, 0, 0, 0, 0,   1.8, 0,   {3, 12,  12, 0,  0}, {3, 12, -12,  0,  0}},
  /* SLEEPING   */ {64, 38, 24,  8, 18, 0, 5, 0, 0,   0,   0,   {3, 12,  90, 0,  0}, {3, 12,  90,  0,  0}},
};

static inline bool due(uint32_t now, uint32_t t) { return (int32_t)(now - t) >= 0; }
static inline float maxf(float a, float b) { return a > b ? a : b; }
static inline int maxi(int a, int b) { return a > b ? a : b; }

KirbyEyes::KirbyEyes(Adafruit_SSD1306& display) : display_(display) {
  for (int i = 0; i < KIRBY_BUILTIN_COUNT; i++) poses_[i] = BUILTIN_POSES[i];
  cur_ = poses_[MOOD_IDLE];
  target_ = poses_[MOOD_IDLE];
}

void KirbyEyes::begin(uint8_t fps) {
  setFPS(fps);
  uint32_t now = millis();
  cur_ = poses_[mood_];
  target_ = poses_[mood_];
  blinkK_ = blinkTarget_ = 1.0f;
  lastFrame_ = now - frameMs_;     // gambar frame pertama langsung
  lastCycle_ = now;
  modeStart_ = now;
  nextBlink_ = now + 2000;
  nextGlance_ = now + 1500;
}

// ---------- pengaturan ----------
void KirbyEyes::setShakeTime(uint16_t ms) { shakeMs_ = ms; }

void KirbyEyes::setEase(float ease) {
  if (ease < 0.02f) ease = 0.02f;
  if (ease > 1.0f) ease = 1.0f;
  ease_ = ease;
}

void KirbyEyes::setFPS(uint8_t fps) {
  if (fps < 1) fps = 1;
  if (fps > 60) fps = 60;
  frameMs_ = 1000 / fps;
}

void KirbyEyes::setAutoBlink(bool on) { autoBlink_ = on; }
void KirbyEyes::setAutoGlance(bool on) { autoGlance_ = on; }

void KirbyEyes::setAutoCycle(bool on, uint16_t intervalMs) {
  autoCycle_ = on;
  cycleMs_ = intervalMs;
  lastCycle_ = millis();
}

void KirbyEyes::setLook(float x, float y) {
  lookOffX_ = x;
  lookOffY_ = y;
}

void KirbyEyes::blink() {
  if (mood_ == MOOD_SLEEPING) return;
  blinkTarget_ = 0.0f;
  blinkReopen_ = millis() + 110;
}

// ---------- ekspresi ----------
void KirbyEyes::setMood(uint8_t mood, bool restart) {
  if (mood >= moodCount_) return;
  if (mood == mood_ && !restart) return;
  mood_ = mood;
  modeStart_ = millis();
  lastCycle_ = modeStart_;
}

int KirbyEyes::addMood(const KirbyPose& p) {
  if (moodCount_ >= KIRBY_MAX_MOODS) return -1;
  poses_[moodCount_] = p;
  return moodCount_++;
}

KirbyPose* KirbyEyes::pose(uint8_t mood) {
  return mood < moodCount_ ? &poses_[mood] : nullptr;
}

// ---------- loop ----------
bool KirbyEyes::update() {
  uint32_t now = millis();
  if ((uint32_t)(now - lastFrame_) < frameMs_) return false;   // non-blocking, tanpa delay()
  lastFrame_ = now;

  behave(now);

  // lerp: tiap nilai ngejar target -> inilah rahasia smooth-nya
  float* c = (float*)&cur_;
  const float* t = (const float*)&target_;
  for (int i = 0; i < POSE_FLOATS; i++) c[i] += (t[i] - c[i]) * ease_;

  // kedip dibikin lebih cepat dari gerakan lain
  blinkK_ += (blinkTarget_ - blinkK_) * 0.5f;

  draw(now);
  return true;
}

void KirbyEyes::behave(uint32_t now) {
  // demo ganti ekspresi otomatis
  if (autoCycle_ && (uint32_t)(now - lastCycle_) > cycleMs_) {
    setMood((mood_ + 1) % moodCount_, true);
  }

  // kedip acak (ga kedip pas tidur)
  if (blinkTarget_ == 0.0f && due(now, blinkReopen_)) blinkTarget_ = 1.0f;
  if (autoBlink_ && mood_ != MOOD_SLEEPING && blinkTarget_ == 1.0f && due(now, nextBlink_)) {
    blinkTarget_ = 0.0f;
    blinkReopen_ = now + 110;
    nextBlink_ = now + random(2200, 5200);
  }
  if (mood_ == MOOD_SLEEPING) blinkTarget_ = 1.0f;

  // lirik acak, kekuatannya diatur field 'glance' tiap pose
  if (due(now, nextGlance_)) {
    glanceX_ = (float)random(-5, 6);
    glanceY_ = (float)random(-2, 3);
    nextGlance_ = now + random(1500, 3500);
  }

  // target = pose dasar + lirikan + arah pandang manual
  target_ = poses_[mood_];
  float g = autoGlance_ ? target_.glance : 0.0f;
  target_.lookX += glanceX_ * g + lookOffX_;
  target_.lookY += glanceY_ * g + lookOffY_;

  // getar cuma nyala sebentar sejak ekspresi dipilih
  if (shakeMs_ > 0 && (uint32_t)(now - modeStart_) > shakeMs_) {
    target_.shakeX = 0;
    target_.shakeY = 0;
  }
}

// ---------- gambar ----------
// Kapsul diputar sebesar ang. k = faktor kedip (1 melek .. 0 tutup),
// kedip digambar pakai "kelopak" putih dari atas & bawah.
void KirbyEyes::drawEye(float x, float y, const KirbyEye& e, float k) {
  const float DEG = 0.0174533f;
  float a  = e.ang * DEG;
  float sa = sinf(a), ca = cosf(a);
  float r  = e.w / 2.0f;
  int   ri = maxi(1, (int)roundf(r));
  float half = maxf(0.0f, e.h / 2.0f - r);   // setengah panjang garis tengah

  float x1 = x + sa * half, y1 = y - ca * half;   // ujung "atas"
  float x2 = x - sa * half, y2 = y + ca * half;   // ujung "bawah"

  int steps = maxi(1, (int)ceilf(half * 2.0f * 1.5f));
  for (int i = 0; i <= steps; i++) {
    float t = (float)i / steps;
    display_.fillCircle((int)roundf(x1 + (x2 - x1) * t),
                        (int)roundf(y1 + (y2 - y1) * t), ri, BLACK);
  }

  // kelopak kedip
  if (k < 0.98f) {
    float extY = fabsf(half * ca) + ri;
    float extX = fabsf(half * sa) + ri;
    int lid = (int)roundf((1.0f - k) * (extY - 1.0f));
    if (lid > 0) {
      int x0 = (int)roundf(x - extX) - 1;
      int wd = (int)(2 * extX) + 3;
      display_.fillRect(x0, (int)roundf(y - extY) - 1, wd, lid + 1, WHITE);   // atas
      display_.fillRect(x0, (int)roundf(y + extY) - lid, wd, lid + 1, WHITE); // bawah
    }
  }
}

void KirbyEyes::draw(uint32_t ms) {
  float bob    = cur_.bounce * sinf(ms / 120.0f);   // goyang naik-turun
  float breath = sinf(ms / 500.0f) * 0.8f;          // napas halus
  float sx     = cur_.shakeX * sinf(ms / 27.0f);    // getar kiri-kanan
  float sy     = cur_.shakeY * sinf(ms / 23.0f);    // getar atas-bawah

  float cx = cur_.cx + sx;
  float cy = cur_.cy + bob + sy;
  float R  = cur_.bodyR + breath;

  display_.clearDisplay();

  // badan + dua tangan
  display_.fillCircle((int)cx, (int)cy, (int)R, WHITE);
  display_.fillCircle((int)(cx - R * 0.95f), (int)(cy + R * 0.30f), (int)cur_.bump, WHITE);
  display_.fillCircle((int)(cx + R * 0.95f), (int)(cy + R * 0.30f), (int)cur_.bump, WHITE);

  // mata kiri & kanan, masing-masing punya parameter sendiri
  float ex = cx + cur_.lookX;
  float ey = cy - 2 + cur_.lookY;
  drawEye(ex - cur_.gap / 2 + cur_.eyeL.dx, ey + cur_.eyeL.dy, cur_.eyeL, blinkK_);
  drawEye(ex + cur_.gap / 2 + cur_.eyeR.dx, ey + cur_.eyeR.dy, cur_.eyeR, blinkK_);

  display_.display();
}
