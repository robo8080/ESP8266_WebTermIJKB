# ESP8266_WebTermIJKB
WebTermはブラウザでIchigoJamのモニタとキーボードの代わりをさせるESP-WROOM-02用スケッチです。 <br>
MixJuiceのファームと入れ替えて使うこともできます。<br>
ただしその場合MixJuiceの機能は失われます。<br>
自己責任においてご利用ください。 <br>
<br>
### 必要な物 ###
* Arduino IDE (1.6.8で動作確認をしました。)<br>
* SPIFFSファイルシステムアップローダー：<https://github.com/esp8266/arduino-esp8266fs-plugin><br>
* ArduinoWebSocketsライブラリ ：<https://github.com/Links2004/arduinoWebSockets><br>
* QueueArray Library For Arduino : <http://playground.arduino.cc/Code/QueueArray>
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


---
WebTermには、Michio OnoさんのMicJackのソースの一部を使わせて頂いております。<br>
MicJack CC BY Michio Ono (Micono Utilities) <http://ijutilities.micutil.com>


