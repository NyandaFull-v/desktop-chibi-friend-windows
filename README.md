# デスクトップちびフレンド v0.4.2

Windows 11 向けのネイティブ Win32 軽量版です。WPF 版 v0.3.16 の画像・設定・所持品を引き継ぎつつ、常駐本体を小さな透過ウィンドウへ置き換えます。

## v0.4.2

- 呼び出しアイテムまたはコインを取得した瞬間に、ダッシュ・落下・登り予約を解除してその場で停止します。

## 構成

- `DesktopChibiFriend.exe`: 常駐本体、設定画面、OBS用ローカル配信ページ
- `characters/chibidaful`: キャラクター差し替え可能な画像・定義
- `settings.json` / `profile.json`: 実行ファイルと同じ場所に保存（配布物には含めない）

## ビルド

Visual Studio 2022 の「C++によるデスクトップ開発」が必要です。

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```
