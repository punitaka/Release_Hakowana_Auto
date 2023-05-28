# イノシシ捕獲　箱罠自動化装置のファイル置き場

## 当レポジトリについて
当レポジトリは、イノシシ捕獲　箱罠自動化装置のプログラムや関連ファイルなどを配置しています。

***

## 開発の背景
関東から九州に移住して、イノシシの猟師さんの活動を初めて知りました。  
イノシシの猟師さんは、各所に仕掛けた箱罠にイノシシが入っているかを定期的に見回っています。  
太い道路沿いなら楽なのでしょうが、そんなところにイノシシは来ません。  
細い山道沿いや、場合によっては車で入れない場所にもイノシシの箱罠があります。  
このような巡回監視の負担を軽減する装置を考えて作ってみました。  
が、コストが見合わないので開発はいったん中止しています。。。

****

## 仕組みの概要
イノシシ箱罠用自動検知システムです。  
WioLTEを用いて箱罠の物体を検知。閾値を超えるとリレーを発動します。  
  
リレーにはモーターを接続して箱罠の扉を閉める想定の作りです。  
リレー発動後はソラコムハーベストにデータを送信します。  
  
ソラコムは、扉が閉まったことをLINEに通知をします。

****

## 仕組みの解説
Youtubeで紹介しています。ぜひご覧ください。 
チャンネル登録も、是非宜しくお願い致します。

◇Youtube再生リスト　【IoT/IT】イノシシ箱罠自動化装置  
https://youtube.com/playlist?list=PLWImbCHDGxLrTCo-s9ceLzzixyG_TZNxH  

***

## 調達した資材やサービス
主に以下の資材やサービスを調達しています。  

- Seed WioLTE  
https://www.seeedstudio.com/Wio-LTE-JP-Version-4G-Cat-1-p-3018.html
- Grove - Ultrasonic Distance Sensor　（超音波センサーモジュール）  
https://www.seeedstudio.com/Grove-Ultrasonic-Distance-Sensor.html
- Grove - Relay　（リレーモジュール）  
https://www.seeedstudio.com/Grove-Relay.html
- 6vギアモーター  
https://akizukidenshi.com/catalog/g/gM-15144/
- ソーラーパネル・チャージコントローラー  
https://www.amazon.co.jp/gp/product/B0146394WQ
- 12Vバッテリー  
https://www.amazon.co.jp/gp/product/B076B9727S
- SORACOM Air 　（通信用SIM）  
https://soracom.jp/services/air/
- SORACOM Harvest　（データ蓄積サービス）  
https://soracom.jp/services/harvest/
- SORACOM Lagoon　（ダッシュボード・アラート発報サービス）  
https://soracom.jp/services/lagoon/

***
