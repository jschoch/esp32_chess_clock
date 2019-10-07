#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include <ButtonDebounce.h>

#define BLYNK_PRINT Serial

#define BLYNK_USE_DIRECT_CONNECT

#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "P6g6IX2WmHZQhue4B0yge3KJ7eM4EU-5";


//int btn1 = 18;
//int btn2 = 19;
ButtonDebounce button1(18, 50);
ButtonDebounce button2(19, 50);
int out1 =0;
int out2 = 0;
int ticks = 0;
int state = 0;
int now = 0;
int start_time = 0;
int last_time = 0;
int game_time = 5* 60 * 1000;
int player = 0;
int player_one_time = game_time;
int player_one_timer = game_time;
int player_two_time = game_time;
int player_two_timer = game_time;
int progress1 = 0;
int progress2 = 0;
int display_mode = 1;
SSD1306Wire  display(0x3c, 4, 15, GEOMETRY_128_32);

void draw_time(){
  
  display.clear();
  
    display.setFont(ArialMT_Plain_10);
    display.drawString(110, 0, String(out1));
    display.drawString(120, 0, String(out2));
    display.setFont(ArialMT_Plain_16);
    display.drawString(5, 0, String(player_two_timer));
    display.drawString(5, 16, String(player_one_timer));
  
  
  display.display();

}

void draw_ready(){
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(5, 0, "READY!");
  display.display();
}
void draw_game(){
  display.clear();
  if(display_mode == 1){
    progress1 = map(player_one_time,0,player_one_timer,0,100);
    progress2 = map(player_two_time,0,player_two_timer,0,100);
    display.drawProgressBar(8, 0, 120, 10, progress2);
    display.drawProgressBar(8, 16, 120, 10, progress1);  
    if(player == 1){
      display.fillCircle(4,8,4);
    }
    if(player == 0){
      display.fillCircle(4,24,4);
    }
  }
  if(display_mode == 0){
    display.setFont(ArialMT_Plain_16);
    
    display.drawString(5, 0, String(player_two_time));
    display.drawString(5, 16, String(player_one_time));
    }
  display.display();
}
void draw_pause(){
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(5, 0, "PAUSED!");
  display.display();
}
void draw_finish(){
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(5, 0, String(player_one_time));
  display.drawString(5, 16, String(player_two_time));
  display.display();
}
void btn_wait(){
  while(out1 != 1){
      //out1 = digitalRead(btn1);
      button1.update();
      if(button1.state() == HIGH){
        out1 = 1;
      }
    }
}
int game_not_done(){
  if (player_one_time < 0){
    state = 4;
    return 0;
  }
  if (player_two_time < 0){
    state = 4;
    return 0;
  }
  return 1;
}
void run_game(){
  if (state == 0 && (out1 == 0 && out2 == 0)){
    btn_wait();
    state = 1;
    delay(500);    
  }
  if (state == 1 && (out1 == 0 || out2 == 0)){
    state = 2;    
    start_time = millis();
    last_time = millis();
    player_one_time = player_one_timer;
    player_two_time = player_two_timer;
  }
  if (state == 2 && game_not_done()){
    now = millis();
    ticks = now - last_time;
    if(player == 0){        
      player_one_time -= ticks;
    }else{
      player_two_time -= ticks;
    }
    last_time = now;
  }
  
  if (state == 2 && player == 0 && out1 == 0 ){
    player = 1;
  }
  if (state == 2 && player == 1 && out2 == 0 ){
    player = 0;
  }
  if (state == 4 && (out1 == 0 && out2 == 0)){
    btn_wait();
    state = 1;    
  }
}

void check_btns(){
  if(button1.state() == HIGH){
    out1 = 1;
  }else{
    out1 = 0;
  }
  if(button2.state() == HIGH){
    out2 = 1;
  }else{
    out2 = 0;
  }
}

BLYNK_WRITE(V0){
  player_one_timer = param.asInt() * 1000 * 60;
  
}
BLYNK_WRITE(V1){

  player_two_timer = param.asInt() * 1000 * 60;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //pinMode(btn1, INPUT_PULLUP);
  //pinMode(btn2, INPUT_PULLUP);
  int r = display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  Serial.print(r);
  Serial.println("Waiting for connections...");

  Blynk.setDeviceName("Blynk");

  Blynk.begin(auth);
  Blynk.virtualWrite(V0,(player_one_timer /60000));
  Blynk.virtualWrite(V1,(player_two_timer / 60000));
  Serial.println(" setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
  //out1 = digitalRead(btn1);
  //out2 = digitalRead(btn2);
  Blynk.run();
  button1.update();
  button2.update();
  check_btns();
  run_game();
  switch (state){
    case 0:
      draw_time();
      break;
    case 1:
      draw_ready();
      break;
    case 2:
      draw_game();
      break;
    case 3:
      draw_pause();
      break;
    case 4: 
      draw_finish();
      break;
  }
}
