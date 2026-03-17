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

[1]:https://deepwiki.com/Sta6leStapler/pandaeSokonium/1-overview

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/Sta6leStapler/pandaeSokonium)
