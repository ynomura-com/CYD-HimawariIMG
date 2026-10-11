// ひまわり(気象庁) トゥルーカラー再現画像を CYD に画像だけ表示する
//
// 画像取得元: 気象庁 防災気象情報 ひまわり画像のタイル
//   https://www.jma.go.jp/bosai/himawari/data/satimg/targetTimes_fd.json
//   https://www.jma.go.jp/bosai/himawari/data/satimg/{base}/fd/{valid}/REP/ETC/{z}/{x}/{y}.jpg
// タイルは地理院地図と同じ Web メルカトル (z/x/y) 方式。
// Wi-Fi 等の設定は include/config.h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "config.h"
#include "coast.h"

static const size_t TILE_BUF_MAX = 48 * 1024;  // タイル1枚の最大サイズ
static const char* BASE_URL = "https://www.jma.go.jp/bosai/himawari/data/satimg/";

TFT_eSPI tft;
WiFiClientSecure secureClient;
uint8_t* tileBuf = nullptr;
String shownTime = "";

// シリアル(と DEBUG_STATUS=1 のとき画面)に状況を出す
static void status(const String& s) {
  Serial.println(s);
#if DEBUG_STATUS
  tft.fillRect(0, 0, tft.width(), 20, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(s, 2, 2, 1);
#endif
}

// 撮影時刻 (UTC, 14桁 "YYYYMMDDhhmmss") を JST の文字列にする
static String toJstString(const String& t) {
  struct tm tmv = {};
  tmv.tm_year = t.substring(0, 4).toInt() - 1900;
  tmv.tm_mon  = t.substring(4, 6).toInt() - 1;
  tmv.tm_mday = t.substring(6, 8).toInt();
  tmv.tm_hour = t.substring(8, 10).toInt();
  tmv.tm_min  = t.substring(10, 12).toInt();
  time_t ts = mktime(&tmv) + 9 * 3600;   // タイムゾーン未設定(UTC)前提で +9時間
  struct tm j;
  gmtime_r(&ts, &j);
  char buf[24];
  snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d",
           j.tm_year + 1900, j.tm_mon + 1, j.tm_mday, j.tm_hour, j.tm_min);
  return String(buf);
}

// 画面上部に撮影日時(JST)を表示
static void drawTimestamp(const String& t) {
#if SHOW_TIMESTAMP
  if (t.length() != 14) return;
  String s = toJstString(t) + " JST";
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int w = tft.textWidth(s, 1) + 8;
  tft.fillRect(0, 0, w, 20, TFT_BLACK);
  tft.drawString(s, 4, 2, 1);
#endif
}

// 海岸線を重ねて描画 (Natural Earth の線データを Web メルカトルで画面座標に変換)
static void drawCoast() {
#if SHOW_COASTLINE
  const float worldPx = 256.0f * (float)(1 << HIMAWARI_ZOOM);
  const int W = tft.width(), H = tft.height();
  bool havePrev = false;
  int px = 0, py = 0;
  for (size_t i = 0; i < COAST_COUNT; i++) {
    int16_t lo = COAST_PTS[2 * i], la = COAST_PTS[2 * i + 1];
    if (lo == COAST_SEP) { havePrev = false; continue; }
    float lon = lo / 100.0f;
    float sinLat = sinf((la / 100.0f) * (float)DEG_TO_RAD);
    float gx = (lon + 180.0f) / 360.0f * worldPx;
    float gy = (0.5f - logf((1.0f + sinLat) / (1.0f - sinLat)) / (4.0f * (float)PI)) * worldPx;
    int sx = IMAGE_OFFSET_X + (int)lroundf((gx - HIMAWARI_TILE_X0 * 256.0f) / JPG_SCALE);
    int sy = IMAGE_OFFSET_Y + (int)lroundf((gy - HIMAWARI_TILE_Y0 * 256.0f) / JPG_SCALE);
    if (havePrev) {
      bool outside = (sx < 0 && px < 0) || (sx >= W && px >= W) ||
                     (sy < 0 && py < 0) || (sy >= H && py >= H);
      if (!outside) tft.drawLine(px, py, sx, sy, COAST_COLOR);
    }
    px = sx; py = sy; havePrev = true;
  }
#endif
}

// TJpg_Decoder → TFT 出力 (範囲外は TFT_eSPI::pushImage が自動クリップ)
static bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (x >= tft.width() || y >= tft.height()) return true;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

static bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  status("WiFi connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) delay(200);
  if (WiFi.status() != WL_CONNECTED) {
    status("WiFi failed (status " + String((int)WiFi.status()) + ")");
    return false;
  }
  status("WiFi OK " + WiFi.localIP().toString());
  return true;
}

// 最新の撮影時刻(UTC, 14桁)を取得
static bool getLatestTime(String& latest) {
  HTTPClient http;
  http.setTimeout(15000);
  if (!http.begin(secureClient, String(BASE_URL) + "targetTimes_fd.json")) {
    status("times: begin failed");
    return false;
  }
  int code = http.GET();
  if (code != 200) {
    status("times: HTTP " + String(code));
    http.end();
    return false;
  }
  String body = http.getString();
  http.end();

  JsonDocument filter;
  filter[0]["validtime"] = true;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body, DeserializationOption::Filter(filter));
  if (err) {
    status(String("times: JSON ") + err.c_str());
    return false;
  }

  latest = "";
  for (JsonObject o : doc.as<JsonArray>()) {
    String v = o["validtime"] | "";
    if (v > latest) latest = v;   // 14桁の数字なので文字列比較で新旧が判定できる
  }
  if (latest.length() != 14) {
    status("times: no valid time");
    return false;
  }
  return true;
}

// タイルを tileBuf に取得。成功時はバイト数、失敗時は -1
static int fetchTile(const String& url) {
  HTTPClient http;
  http.useHTTP10(true);          // chunked 転送を避ける
  http.setTimeout(15000);
  if (!http.begin(secureClient, url)) return -1;
  int code = http.GET();
  if (code != 200) {
    Serial.printf("tile HTTP %d: %s\n", code, url.c_str());
    http.end();
    return -1;
  }

  int expected = http.getSize();
  WiFiClient* s = http.getStreamPtr();
  size_t pos = 0;
  uint32_t last = millis();
  while ((http.connected() || s->available()) && millis() - last < 10000) {
    int a = s->available();
    if (a > 0) {
      size_t room = TILE_BUF_MAX - pos;
      if (room == 0) { http.end(); return -1; }
      int n = s->readBytes(tileBuf + pos, min((size_t)a, room));
      pos += n;
      last = millis();
      if (expected > 0 && (int)pos >= expected) break;
    } else {
      delay(1);
    }
  }
  http.end();
  if (pos < 4 || tileBuf[0] != 0xFF || tileBuf[1] != 0xD8) return -1;  // JPEG以外は不正
  return (int)pos;
}

static bool updateImage() {
  status("Checking latest time...");
  String t;
  if (!getLatestTime(t)) return false;
  if (t == shownTime) {
    Serial.println("no new image");
    drawTimestamp(shownTime);    // DEBUG_STATUS で上書きされた分を戻す
    return true;
  }
  status("Latest " + t);

  const int tilePx = 256 / JPG_SCALE;
  bool allOk = true;
  int n = 0, total = HIMAWARI_TILE_COLS * HIMAWARI_TILE_ROWS;
  for (int r = 0; r < HIMAWARI_TILE_ROWS; r++) {
    for (int c = 0; c < HIMAWARI_TILE_COLS; c++) {
      String url = String(BASE_URL) + t + "/fd/" + t + "/REP/ETC/" + HIMAWARI_ZOOM + "/" +
                   (HIMAWARI_TILE_X0 + c) + "/" + (HIMAWARI_TILE_Y0 + r) + ".jpg";
      status("Tile " + String(++n) + "/" + String(total));
      int len = fetchTile(url);
      if (len < 0) { allOk = false; Serial.println("fail: " + url); continue; }
      TJpgDec.drawJpg(IMAGE_OFFSET_X + c * tilePx, IMAGE_OFFSET_Y + r * tilePx, tileBuf, len);
    }
  }
  drawCoast();
  if (allOk) {
    shownTime = t;
    drawTimestamp(t);
    Serial.println("done: " + t);
  } else {
    status("Some tiles failed");
  }
  return allOk;
}

void setup() {
  Serial.begin(115200);
  delay(1500);                   // モニタが開くのを待つ
  Serial.println("\n=== Himawari CYD start ===");

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);            // 横向き 320x240 (上下逆なら 3 に)
  tft.fillScreen(TFT_BLACK);
  status("Display OK");

  TJpgDec.setJpgScale(JPG_SCALE);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tftOutput);

  secureClient.setInsecure();    // 証明書検証なし (表示専用のため)
  tileBuf = (uint8_t*)malloc(TILE_BUF_MAX);
  if (!tileBuf) status("malloc failed");
}

void loop() {
  bool ok = tileBuf && connectWiFi() && updateImage();
  Serial.printf("next update in %lu s (free heap %u)\n",
                (ok ? REFRESH_MS : RETRY_MS) / 1000, ESP.getFreeHeap());
  delay(ok ? REFRESH_MS : RETRY_MS);
}
