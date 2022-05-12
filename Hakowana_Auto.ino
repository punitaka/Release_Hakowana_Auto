#include <WioLTEforArduino.h>
#include <stdio.h>
#include "Ultrasonic.h"

//------------------------------------------------
// 基本設定
//------------------------------------------------
// SIM通信受信タイムアウト時間(ミリ秒)
#define RECEIVE_TIMEOUT (10000)
// 距離センサー閾値外カウント　この値以上でモーター発動
#define ACTIVATION_COUNT 5

//------------------------------------------------
// 距離センサー設定
//------------------------------------------------
// 距離の閾値(下限cm)　これを下回ると発報
#define THRESHOLD_DISTANCE_UNDER        65
// 距離の閾値(上限cm)　これを上回ると発報
#define THRESHOLD_DISTANCE_UPPER        9999

//------------------------------------------------
// ピン設定
//------------------------------------------------
// 超音波距離センサーのWioLTE接続ピン
#define ULTRASONIC_PIN  (WIOLTE_D38)
// リレーのWioLTE接続ピン
#define RELAY_PIN  (WIOLTE_D20)

//------------------------------------------------
// LED色設定
//------------------------------------------------
#define COLOR_NONE      0, 0, 0
#define COLOR_WHITE   10, 10, 10
#define COLOR_RED       10, 0, 0
#define COLOR_GREEN   0, 10, 0
#define COLOR_BLUE      0, 0, 10
#define COLOR_YELLOW    10, 10, 0
#define COLOR_CYAN          0, 10, 10
#define COLOR_MAGENTA   10, 0, 10

//------------------------------------------------
// 状態ステータス
//------------------------------------------------
// 起動中
#define STS_BOOT  100
// 初回通信中
#define STS_TRANSMIT  200
// 検知中
#define STS_DETECT  300
// 追加検知中
#define STS_EVALUATE  400
// モーター作動済
#define STS_ACTIVATE  500
// エラー
#define STS_ERR  900

  
//------------------------------------------------
// オブジェクト定義・クラス変数
//------------------------------------------------
// WioLTE定義
WioLTE Wio;
// 超音波距離センサー定義
Ultrasonic UltrasonicRanger(ULTRASONIC_PIN);
// 状態ステータス
unsigned int iG_Status = 0;
// 繰り返し処理のインターバル(ミリ秒)
unsigned long lG_Interval = 20000;
// 連続検知カウント用変数
unsigned int iG_DetectCnt = 0;


//------------------------------------------------
//  概要　：初期化処理
//  詳細　：初期化処理
//  引数   ：なし
//  戻り値：なし
//  本体LED制御：
//  　　　　起動時：白
//  　　　　起動後：無色
//------------------------------------------------
void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Grove Power supply ON.");
  Wio.PowerSupplyGrove(true);
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);

  // リレーピン設定・初期化
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // ステータス設定
  setStatusValue(STS_BOOT);
  
  delay(2000);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    // ステータス設定
    setStatusValue(STS_ERR);
    SerialUSB.println("### ERROR! ###");
    return;
  }

  // ステータス設定
  setStatusValue(STS_TRANSMIT);
  SerialUSB.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    // ステータス設定
    setStatusValue(STS_ERR);
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Setup completed.");
}

//------------------------------------------------
//  概要　：メインループ
//  詳細　：メイン処理
//  引数   ：なし
//  戻り値：なし
//  本体LED制御：
//  　　　　なし
//------------------------------------------------
void loop() {
  // soracomハーベスト送信用文字列
  char data[100];

  // 超音波センサーで距離測定
  long distance = measure_distance();
  
  // 既にモーター作動済みモードならステータスはそのまま
  // モーター作動済み以外の場合には、評価続行
  if(iG_Status != STS_ACTIVATE)
  {
        // ステータス設定
        setStatusValue(STS_DETECT);  
    
        if(distance == -1)
        {
            // 距離が閾値内の時
            // 検知カウントをクリア
            iG_DetectCnt = 0;
            setStatusValue(STS_DETECT);  
            // 繰り返しへ
            goto continu;
        }
      
        // 距離が閾値外の時
        // 検知カウントをインクリメント
        iG_DetectCnt++;
        
        if (iG_DetectCnt < ACTIVATION_COUNT)
        {
           // 距離センサー閾値外カウント以内の時は、評価モードに移行
            setStatusValue(STS_EVALUATE);  
            // 繰り返しへ
            goto continu;
        }

    // 距離センサー閾値外カウント以上となったので、モーター作動モードに移行
    // リレーモジュールを開放しモーター作動
    activate_relay(20000);

    // ステータスをモーター作動済みに移行
    setStatusValue(STS_ACTIVATE);  
  }

  // ソラコムハーベストにする文字列を作成
  sprintf(data,"{\"alert_flg\":%d,\"distance\":%ld}", 100, distance);

  ////////////////////////////////////////////////////////
  /// ソラコムへ送信する処理 Start ///
  ////////////////////////////////////////////////////////
  SerialUSB.println("### Open.");
  int connectId;
  connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

  SerialUSB.println("### Send.");
  SerialUSB.print("Send:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }

  SerialUSB.println("### Receive.");
  int length;
  length = Wio.SocketReceive(connectId, data, sizeof (data), RECEIVE_TIMEOUT);
  if (length < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }
  if (length == 0) {
    SerialUSB.println("### RECEIVE TIMEOUT! ###");
    goto err_close;
  }
  SerialUSB.print("Receive:");
  SerialUSB.print(data);
  SerialUSB.println("");

err_close:
  SerialUSB.println("### Close.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  //////////////////////////////////////////////////////
  /// ソラコムへ送信する処理 End ///
  //////////////////////////////////////////////////////

err:
continu:
  // 次の繰り返し
  delay(lG_Interval);
}

////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------
//  概要　：グローバル変数にステータスをセットし関連処理を行う
//  詳細　：グローバル変数　iG_Status　に引数の値をセットし、関連処理を実施。
//                併せて、setLEDbyStatus() 関数を呼び出しステータスに応じたＬＥＤ色をセットする
//  引数   ：unsigned int prmiStatus : 状態ステータスを指定する
//                以下のいずれかを想定。これ以外は処理をせず終了
//                    STS_BOOT　        ：　起動中
//                    STS_TRANSMIT　 ：　初回通信中
//                    STS_DETECT　    ：　検知中
//                    STS_EVALUATE　：　追加検知中
//                    STS_ACTIVATE　：　モーター作動済
//                    STS_ERR　          ：　エラー
//  戻り値：なし
//------------------------------------------------
void setStatusValue(unsigned int prmiStatus)
{
    // 引数のステータスをセット
    iG_Status = prmiStatus;

    // ステータスに応じた関連処理を実行
    switch (iG_Status) {
        case STS_BOOT:
            lG_Interval = 20000;
            break;
        case STS_TRANSMIT:
            lG_Interval = 20000;
            break;
        case STS_DETECT:
            lG_Interval = 10000;
            break;
        case STS_EVALUATE:
            lG_Interval = 1000;
            break;
        case STS_ACTIVATE:
            lG_Interval = 600000;
            break;
        case STS_ERR:
            lG_Interval = 20000;
            break;
        default:
            lG_Interval = 30000;
            break;
    }
    
    // LEDの色を設定
    setLEDbyStatus();
}

//------------------------------------------------
//  概要　：ステータスに応じたLEDの色を設定する
//  詳細　：グローバル変数　iG_Status　を評価してLED色を設定する
//  引数   ：なし
//  戻り値：なし
//  本体LED制御：
//　　　色　　ステータス値
//　　　-----------------------------------------------
//　　　COLOR_WHITE　　STS_BOOT（起動中）　　 
//　　　COLOR_CYAN        STS_TRANSMIT（初回通信中）　　 
//　　　COLOR_NONE　　 STS_DETECT（検知中）　　
//　　　COLOR_GREEN　  STS_EVALUATE（評価中）　　 
//　　　COLOR_YELLOW　STS_ACTIVATE（モーター作動済）　　  
//　　　COLOR_RED　　　STS_ERR（エラー）　　  
//------------------------------------------------
void setLEDbyStatus()
{

    switch (iG_Status) {
        case STS_BOOT:
            Wio.LedSetRGB(COLOR_WHITE);
            break;
        case STS_TRANSMIT:
            Wio.LedSetRGB(COLOR_CYAN);
            break;
        case STS_DETECT:
            Wio.LedSetRGB(COLOR_NONE);
            break;
        case STS_EVALUATE:
            Wio.LedSetRGB(COLOR_GREEN);
            break;
        case STS_ACTIVATE:
            Wio.LedSetRGB(COLOR_YELLOW);
            break;
        case STS_ERR:
            Wio.LedSetRGB(COLOR_RED);
            break;
        default:
            Wio.LedSetRGB(COLOR_RED);
            break;
    }
}

//------------------------------------------------
//  概要　：超音波距離センサーの測定と測定結果を返す
//  詳細　：設定した閾値範囲内の場合には、-1を返す。
//  引数   ：なし
//  戻り値：long 型　測定距離値(閾値範囲内の場合には-1)
//------------------------------------------------
long measure_distance()
{
    long distance;
    distance = UltrasonicRanger.MeasureInCentimeters();

    if(distance < THRESHOLD_DISTANCE_UNDER or THRESHOLD_DISTANCE_UPPER < distance)
    {
        SerialUSB.print("return distance:");
        SerialUSB.print(distance);
        SerialUSB.println("[cm]");
        return distance;
    }
    else
    {
        return -1;
    }
}

//------------------------------------------------
//  概要　：リレーモジュールを指定時間ONにする
//  詳細　：
//  引数   ：unsigned int prmiWaitms ： リレーをONにするミリ秒
//  戻り値：なし
//------------------------------------------------
void activate_relay(unsigned int prmiWaitms)
{
      digitalWrite(RELAY_PIN, HIGH);
      delay(prmiWaitms);
      digitalWrite(RELAY_PIN, LOW);
      delay(1000);
}
////////////////////////////////////////////////////////////////////////////////////////
