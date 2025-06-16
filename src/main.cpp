#include "mbed.h"

//この基板のCANIDはここで定義している
constexpr unsigned int CANID = 5;

BufferedSerial pc{USBTX, USBRX, 115200};

CAN can(PA_11,PA_12,1e6);
CANMessage msg;

//ReadDataはメインマイコンの送信前の値を復元して代入する。
int16_t ReadData[4] = {0,0,0,0};
//Outputは最終的な出力値を代入する。
float Output[8] = {0,0,0,0,0,0,0,0};

//P_FWR→Pポートの前転信号出力
//P_REV→Pポートの後転信号出力
PwmOut A_FWR(PA_3);
PwmOut A_REV(PA_4);
PwmOut B_FWR(PA_6);
PwmOut B_REV(PA_7);
PwmOut C_FWR(PA_10);
PwmOut C_REV(PA_9);
PwmOut D_FWR(PB_5);
PwmOut D_REV(PB_4);

//何かしらのCAN通信を受け取っている時に光らすLED
DigitalOut Indicator(PB_1);

int main(){
  constexpr auto timeout = 50ms;
  while(1){
    auto no = HighResClock::now();
    static auto last = now;
    //CANの受け取り処理
    if(can.read(msg) && msg.id == CANID){
      for(int i = 0; i<4; i++){
        ReadData[i] = msg.data[2*i+1] << 8 | msg.data[2*i];
      }
      last = now;
    } else {
      for(int i=0; i<4; i++){
        ReadData[i] = 0;
      }
    }
    //信号の整形処理
    for(int i=0; i<4; i++){
      if(ReadData[i]>0){
        Output[2*i] = abs(ReadData[i]) / 32767.0;
        Output[2*i+1] = 0;
      }else if(ReadData[i]<0){
        Output[2*i] = 0;
        Output[2*i+1] = abs(ReadData[i]) / 32767.0;
      }else{
        Output[2*i] = 0;
        Output[2*i+1] = 0;
      }
    }
    //タイムアウト処理
    if (now - last > timeout){
      memset(ReadData, 0, sizeof(ReadData));
    }
    //出力処理
    A_FWR = Output[0];
    A_REV = Output[1];
    B_FWR = Output[2];
    B_REV = Output[3];
    C_FWR = Output[4];
    C_REV = Output[5];
    D_FWR = Output[6];
    D_REV = Output[7];
    Indicator = can.read(msg);
  }
}
