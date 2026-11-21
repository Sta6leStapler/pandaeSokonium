# pandaeSokonium

**pandaeSokonium** は、C++ と SFML による倉庫番ゲームです。
問題の自動生成機能を備えています。

[DeepWikiによるドキュメントはこちら][1]

---

## 倉庫番のルール

プレイヤーを操作して、フィールド上の全ての荷物をゴールに運搬するゲームです。
プレイヤーは荷物を押すことができますが、荷物を引っ張ることはできません。
2つ以上1列に並んだ荷物を同時に押すことはできません。

---

## デモ動画 (Demo Video)

ゲームの各種機能や詳しい操作方法については、以下のデモ動画をご覧ください。

[pandaeSokonium 機能紹介・操作デモ動画](https://youtu.be/fpuK2OCWPu8)

---

## 各種機能

- 日本語・英語の多言語対応（ポーズメニューからリアルタイム切替可能）
- キーボード入力によるプレイヤーの操作
- マウス入力によるプレイヤーの操作
- Undo / Redo 機能
- テキストファイルによる盤面の読み込み
- 自動生成された問題のセーブ機能
- プレイヤーの操作ログの出力機能
- 簡易的な詰み検出
- 荷物の運搬可能なマスを表示するアシスト機能
- 盤面のスナップショットを保存し、後でロードできる機能
- GUI付き盤面編集機能

---

## 問題の生成機能について

私の卒業研究で開発した手法を用いています。
生成される問題は必ず解くことができます。
10*10程度までの盤面サイズであれば、1分以内に問題を作成できます。
生成速度はかなり早めですが、その分生成される問題の品質をいくらか犠牲にしています。
生成速度を活かして何度も問題を生成させ、高品質な問題を取捨選択するように使ってみてください。

- 第51回GI研究発表会 [移動軌跡と未使用セルの壁化の2段階による効率的な倉庫番問題の生成法](https://ipsj.ixsq.nii.ac.jp/records/232903)

---

## 開発環境と依存関係

- **言語**: Visual C++
- **ライブラリ**:
  - [SFML](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library)
  - [TGUI](https://tgui.eu/) (SFML 用 GUI ライブラリ)
- **開発環境**: Visual Studio 2022

---

## ビルド手順（Visual Studio）

1. SFMLとTGUIをダウンロードし、プロジェクトルートの../include/と../lib/に配置
2. `Sokoban-app.sln` を Visual Studio で開く
3. ビルド構成を選択(Debug/Release、Win32/x64) 
4. ビルド＆実行（`Ctrl+F5`）

## 必要なライブラリファイル
### Debug構成
- sfml-system-d.lib, sfml-window-d.lib, sfml-graphics-d.lib, sfml-audio-d.lib, sfml-network-d.lib
tgui-d.lib
### Release構成
- sfml-system.lib, sfml-window.lib, sfml-graphics.lib, sfml-audio.lib, sfml-network.lib
tgui.lib

---

## 実装予定/実装中の機能

- アニメーションのイージング（加減速）バリエーションの追加
- サウンド
- プレイログの可視化ビューワーの実装（UI 枠組みは実装済み）

## リソースと保存先（ポータブル方式）

- リソースは実行ファイルと同じディレクトリの `Assets` から読み込みます。
  起動時の作業ディレクトリには依存しません。
- Visual Studioのビルド後に、リソースと実行時DLLを出力ディレクトリへコピーします。
  配布する場合は実行ファイル・DLL・`Assets` を一緒に配置してください。
- 新しい盤面の保存ダイアログは `UserData/Boards`、ログは `UserData/Logs` を
  初期保存先にします。`UserData` は実行ファイルと同じディレクトリに自動作成します。
  ダイアログで別の場所を選ぶこともできます。
- 盤面一覧には `Assets/Boards` の `[同梱]` と `UserData/Boards` の `[保存]` を
  両方表示します。同名でも別の盤面として選択できます。別の場所に保存した盤面を
  一覧に出す場合は `UserData/Boards` へコピーしてください。
- 既存の盤面・ログをユーザーデータ領域へ移動・自動コピーしたり、削除したりしません。既存の `Save` や
  `Assets/Logs` のデータもそのまま残ります。現在のスナップショットは従来どおり
  メモリ上の保持で、今回ファイル保存機能は追加していません。
- 実行ファイル付近に書き込み権限が必要です。読み取り専用の場所から実行する場合は、
  アプリ一式を書き込めるフォルダーへ移してください。保存先の作成やリソースの
  初期読み込みに失敗すると、原因をコンソールに出力して起動を中止します。
  実行中の保存失敗は画面に通知し、エディターの変更を保存済み扱いにしません。
- 自動出力ログは、同じ秒のログが既にあれば番号を付け、既存ログを上書きしません。
- macOSの `.app` 配置では `Contents/Resources/Assets` を読み、
  `.app` と同じディレクトリの `UserData` に保存する構成です。
  Mac/Linuxのビルド・パッケージ設定と実機検証は別途必要です。

### パス管理のテスト

Visual Studioの「x64 Native Tools Command Prompt」でプロジェクトルートから実行します。
テスト実行ファイルは任意の作業用ディレクトリに作成してください。

```bat
cl /nologo /std:c++17 /EHsc /utf-8 Sokoban-app\AppPaths.cpp Sokoban-app\tests\AppPathsTests.cpp /Fe:AppPathsTests.exe
AppPathsTests.exe
```

作業ディレクトリ非依存、日本語パス、同名盤面の識別、ログ名の衝突、
既存盤面の保持、読み書き失敗、`.app` の配置を検証します。
今回変更した既存C++ファイルはBOM付きUTF-8に変換し、Visual Studioが各ファイルの
文字コードを識別できるようにしています。変更したソースの文字列には
`/execution-charset:utf-8` を指定します。未変更の既存ソースは従来の文字コードを維持しています。

[1]:https://deepwiki.com/Sta6leStapler/pandaeSokonium/1-overview

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/Sta6leStapler/pandaeSokonium)
