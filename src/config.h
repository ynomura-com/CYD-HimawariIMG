#pragma once

// ====== Wi-Fi ======
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"

// ====== 液晶の個体差 ======
// 画像が白黒反転(ネガ)して見える場合は 1
#define DISPLAY_INVERT 0
// 赤と青が入れ替わって見える場合(空が赤っぽい、海岸線が水色になる等)は 1
#define DISPLAY_SWAP_RB 0

// ====== 画像の種類 ======
// 気象庁タイルのレイヤ名 (…/{valid}/{レイヤ}/{z}/{x}/{y}.jpg)
#define LAYER_COLOR "REP/ETC"   // トゥルーカラー再現画像
#define LAYER_IR    "B13/TBB"   // 赤外画像
#define START_IN_IR 0           // 1: 起動時から赤外画像

// ====== タッチ操作 ======
// 1: 画面をタッチするたびに トゥルーカラー ⇔ 赤外 を切り替え
#define ENABLE_TOUCH 1
// CYD (ESP32-2432S028) のタッチ(XPT2046)のピン。通常は変更不要
#define TOUCH_CLK  25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32
#define TOUCH_CS   33
#define TOUCH_IRQ  36

// ====== 表示 ======
// 1: 画面上部に撮影日時(JST)を表示 (赤外のときは末尾に IR)
#define SHOW_TIMESTAMP 1

// 1: 海岸線を重ねて表示
#define SHOW_COASTLINE 1
#define COAST_COLOR    TFT_YELLOW

// 動作確認用 1: 接続状況などを画面左上にも文字で表示 / 0: 表示しない
#define DEBUG_STATUS 0

// ====== 表示範囲 (フルディスク画像は zoom 3〜5) ======
#define HIMAWARI_ZOOM      4
#define HIMAWARI_TILE_X0   12
#define HIMAWARI_TILE_Y0   5
#define HIMAWARI_TILE_COLS 3
#define HIMAWARI_TILE_ROWS 3

// JPEG縮小率 (1,2,4,8)。タイル1枚は 256/JPG_SCALE px 四方
#define JPG_SCALE 2

// 画面左上に対するモザイク画像の位置 (px)
#define IMAGE_OFFSET_X (-64)
#define IMAGE_OFFSET_Y (-64)

// ====== 更新間隔 ======
#define REFRESH_MS (30UL * 60UL * 1000UL)  // 通常: 30分
#define RETRY_MS   (60UL * 1000UL)         // 失敗時: 1分
