# ESP8266_WebTermIJKB
ブラウザでIchigoJamのモニタとキーボードの代わりをさせるESP-WROOM-02用スケッチです。 <br>
MixJuiceのファームと入れ替えて使うこともできます。<br>
ただしその場合MixJuiceの機能は失われます。<br>
自己責任においてご利用ください。 <br>
<br>
### 必要な物 ###
* Arduino IDE (1.6.8で動作確認をしました。)<br>
* SPIFFSファイルシステムアップローダー：<https://github.com/esp8266/arduino-esp8266fs-plugin><br>
* rduinoWebSocketsライブラリ ：<https://github.com/Links2004/arduinoWebSockets><br>
* IchigoJam用の TrueTypeフォント「IchigoJam-1.2.ttf」：<http://15jamrecipe.jimdo.com/ツール/フォント-truetype/ ><br>


### インストール手順 ###
 1.Arduino IDEをインストールします。<br>
 2.SPIFFSファイルシステムアップローダーをインストールします。<br>
ここを参考にしてください。<https://www.mgo-tec.com/spiffs-filesystem-uploader01-html><br>
 3.arduinoWebSocketsライブラリをインストールします。<br>
 4.IchigoJam-1.2.ttfをダウンロードしてスケッチのdataフォルダの中に入れて下さい。<br>
 5.スケッチのssidとpasswordを自分の環境に合わせて書き換えて下さい。<br>
 6.スケッチをコンパイルしてESP-WROOM-02に書き込みます。<br>
 7.SPIFFSファイルシステムアップローダーでdataフォルダの中のファイルをSPIFSに書き込みます。<br>



### 接続 ###
ESP-WROOM-02のTXとIchigoJamのRX <br>
ESP-WROOM-02のRXとIchigoJamのTX <br>
ESP-WROOM-02のGNDとIchigoJamのGND <br>
をそれぞれ接続して下さい。 <br>
ESP-WROOM-02にLEDを付けておくと通信状態を確認できます。<br>
MixJuiceの回路を参考にして下さい。<http://mixjuice.shizentai.jp/><br>


### 使い方 ###
 1.IchigoJamとESP-WROOM-02の電源を同時に入れてください。
 2.アクセスポイントに接続するとLED1が点灯します。
 3.ブラウザで、http://esp8266.local にアクセスします。



---
WebTermには、Michio OnoさんのMicJackのソースの一部を使わせて頂いております。<br>
MicJack CC BY Michio Ono (Micono Utilities) <http://ijutilities.micutil.com>


