#pragma once

// ====== Wi-Fi ======
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"

// ====== 表示 ======
// 1: 画面上部に撮影日時(JST)を表示
#define SHOW_TIMESTAMP 1

// 1: 海岸線を重ねて表示
#define SHOW_COASTLINE 1
#define COAST_COLOR    TFT_YELLOW

// 動作確認用 1: 接続状況などを画面左上にも文字で表示 / 0: 表示しない
#define DEBUG_STATUS 0

// ====== 表示範囲 (フルディスク画像は zoom 3〜5) ======
// 既定: z=4, x=12..14, y=5..6 → 概ね東経101〜157度 / 北緯22〜53度 (日本付近)
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
#define REFRESH_MS (10UL * 60UL * 1000UL)  // 通常: 30分
#define RETRY_MS   (60UL * 1000UL)         // 失敗時: 1分
