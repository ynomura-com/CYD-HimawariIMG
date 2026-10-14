# CYD-HimawariIMG

気象庁が提供している気象衛星ひまわりの画像（30分毎、トゥルーカラー再現画像）をCYD（ESP32-2432S028）に画像だけを表示するアプリ。（JMAの画像配信サイトから。）
　Visual Studio Code + PlatforamIO の環境でビルドしてアップロードします。
## 設定
`config.h` の `WIFI_SSID` と `WIFI_PASS`を自分の環境に書き換えます。

## 動作
* 気象庁が防災気象情報で配信している画像タイルを使います。
* 最新の撮影時刻を `targetTimes_fd.json` で調べ、トゥルーカラー再現画像（REP/ETC）のタイル6枚を取得して、画面にそのまま描画します。
* 30分ごとに更新し、新しい画像が来ていなければ再描画しません。失敗したら1分後に再試行します。
* 既定の表示範囲は日本付近（東経約101〜157度、北緯約22〜53度）です。

## 表示範囲を変える

* `ZOOM`、`TILE_X0`、`TILE_Y0`、`TILE_COLS`、`TILE_ROWS`、`OFFSET_X`、`OFFSET_Y` で調整できます。
* フルディスクのズームは 3〜5 です。

### 日本と南太平洋を見る設定
台風発生の状況などを確認
```cpp
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
```
### 岩手県を中心とした東北地方の画像を表示する設定
```cpp
// ====== 表示範囲 (フルディスク画像は zoom 3〜5) ======
// 既定: z=4, x=12..14, y=5..6 → 概ね東経101〜157度 / 北緯22〜53度 (日本付近)
#define HIMAWARI_ZOOM      5
#define HIMAWARI_TILE_X0   27
#define HIMAWARI_TILE_Y0   11
#define HIMAWARI_TILE_COLS 3
#define HIMAWARI_TILE_ROWS 2

// JPEG縮小率 (1,2,4,8)。タイル1枚は 256/JPG_SCALE px 四方
#define JPG_SCALE 1

// 画面左上に対するモザイク画像の位置 (px)
#define IMAGE_OFFSET_X (-236)
#define IMAGE_OFFSET_Y (-174)
```

## 画面左上に、撮影日時を日本時間で表示
（例：2026/10/11 14:30 JST）。

### 表示のタイミング
 6枚のタイルをすべて描き終えた後に表示します。文字の背景は黒の帯にしてあるので、雲の上でも読めます。
###時刻の換算
 気象庁の撮影時刻は UTC なので、+9時間して JST に直しています。例えば 20261011053000 なら 2026/10/11 14:30 JST になります。
### 新しい画像がない回
 日時の表示はそのまま残ります。
### 設定
 `config.h` に `SHOW_TIMESTAMP` を追加しました。日時を消したいときは `0` にしてください。
### 日時の位置や文字サイズ、色を変る
main.cpp の drawTimestamp() の中で調整できます。
###既定値の変更
 画像だけの表示にするため、`DEBUG_STATUS` の既定を `0` に変えました。接続状況の確認が必要なときだけ `1` にしてください。
 。
