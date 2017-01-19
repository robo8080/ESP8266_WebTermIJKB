# ESP8266_WebTermIJKB
WebTermはブラウザでIchigoJamのモニタとキーボードの代わりをさせるESP-WROOM-02用スケッチです。 <br>
MixJuiceのファームと入れ替えて使うこともできます。<br>
ただしその場合MixJuiceの機能は失われます。<br>
自己責任においてご利用ください。 <br>
<br>
### 特徴 ###
* IchigoJamにモニタやキーボードを付けなくてもiPad上などのブラウザからIchigoJamを操作できます。<br>
* テキストファイルからプログラムをIchigoJamにアップロードできます。<br>
* MixJuiceの"MJ GET"、"MJ POST"コマンドが使えます。
* MixJuiceの"MJ GET"コマンドが使えるのでMixJuice対応のサイトから直接プログラムをダウンロードできます。<br>
* エスケープシーケンスでカラー表示ができます。<br>
* OTAに対応しているのでWiFi経由でファームを書き換えらえます。<br>


### 構成 ###
* IchigoJam + ESP-WROOM-02(MixJuice)<br>


### インストールするために必要な物 ###
* Arduino IDE (1.6.8で動作確認をしました。)<br>
* SPIFFSファイルシステムアップローダー：<https://github.com/esp8266/arduino-esp8266fs-plugin><br>
* ArduinoWebSocketsライブラリ ：<https://github.com/Links2004/arduinoWebSockets><br>
* QueueArray Library For Arduino : <http://playground.arduino.cc/Code/QueueArray><br>
* IchigoJam用の TrueTypeフォント「IchigoJam-1.2.ttf」：<http://15jamrecipe.jimdo.com/ツール/フォント-truetype/><br>


### インストール手順 ###
 1. Arduino IDEをインストールします。<br>
 2. SPIFFSファイルシステムアップローダーをインストールします。<br>
ここを参考にしてください。<https://www.mgo-tec.com/spiffs-filesystem-uploader01-html><br>
 3. arduinoWebSocketsライブラリと、QueueArray Library For Arduinoをインストールします。<br>
 4. IchigoJam-1.2.ttfをダウンロードして、スケッチのdataフォルダの中に入れて下さい。<br>
 5. スケッチのssidとpasswordを自分の環境に合わせて書き換えて下さい。<br>
 6. スケッチをコンパイルしてESP-WROOM-02に書き込みます。<br>
 7. SPIFFSファイルシステムアップローダーでdataフォルダの中のファイルをSPIFSに書き込みます。<br>
 8. Arduino IDEのシリアルモニタを起動してアクセスポイントに正常に接続出来るか確認して下さい。（通信速度19200bps）<br>
正常に接続出来ればESP-WROOM-02に割り振られたIPアドレスが表示されます。メモしておいてください。<br>

### 接続 ###
ESP-WROOM-02のTXとIchigoJamのRX <br>
ESP-WROOM-02のRXとIchigoJamのTX <br>
ESP-WROOM-02のGNDとIchigoJamのGND <br>
をそれぞれ接続して下さい。 <br>
ESP-WROOM-02にLEDを付けておくと通信状態を確認できます。<br>
MixJuiceの回路を参考にして下さい。<http://mixjuice.shizentai.jp/><br>


### 使い方 ###
 1. IchigoJamとESP-WROOM-02の電源を同時に入れてください。<br>
 2. アクセスポイントに接続するとLED1が点灯します。<br>
 3. ブラウザで、http://esp8266.local 又は http://esp8266.local/edit.htm にアクセスします。<br>
 上手く接続できないときは、http://192.168.xxx.xxx の様に、IPアドレスで直接アクセスして下さい。<br>


### 使用例（動画） ###
*  [iPad + Safari](https://youtu.be/f58g2r6TK8s "Title")<br>
*  [Windows10 + Edge](https://youtu.be/etVt1T4E4ng "Title")<br>
*  [iPhone + Chrome](https://youtu.be/XS2PH5S9e08 "Title") (Michio Onoさん作成動画)<br>


### 制限事項 ###
* Ichigojamのスクリーンエディタ機能は使えません。<br>
* PEEK/POKE命令を使ったプログラムは正常に動かない可能性があります。<br>
* SCROLLの左右スクロールには対応していません。<br>
* 一度に大量のデータを画面表示するとデータを取りこぼすことがあります。<br>
* その他、汎用のシリアルターミナルソフトを使った時と同じ制限があります。<br>

---
WebTermには、Michio OnoさんのMicJackのソースの一部を使わせて頂いております。<br>
MicJack CC BY Michio Ono (Micono Utilities) <http://ijutilities.micutil.com>


