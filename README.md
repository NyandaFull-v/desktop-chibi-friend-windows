# デスクトップちびフレンド v0.4.1

> 公開されていますが、オープンソース／フリー素材ではありません。コードとキャラクター画像を含む全ファイルは All Rights Reserved です。利用条件は [LICENSE](LICENSE) を確認してください。

Windows 11 向けのネイティブ Win32 軽量版です。WPF 版 v0.3.16 の画像・設定・所持品を引き継ぎつつ、常駐本体を小さな透過ウィンドウへ置き換えます。

## 構成

- `DesktopChibiFriend.exe`: 常駐本体、設定画面、OBS用ローカル配信ページ
- `characters/chibidaful`: キャラクター差し替え可能な画像・定義
- `settings.json` / `profile.json`: 実行ファイルと同じ場所に保存（配布物には含めない）

詳しい操作方法は [README.txt](README.txt)、プライバシー方針は [PRIVACY.md](PRIVACY.md) を確認してください。

## クレジット

- キャラクター名：ちび堕ふる
- キャラクター原案・著作権：権利者本人
- 元立ち絵制作：やたノおけい様（権利者提供情報に基づく表記）
- アプリ名：デスクトップちびフレンド

## ビルド

Visual Studio 2022 の「C++によるデスクトップ開発」が必要です。

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```
