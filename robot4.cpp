#include "BrickPi3.h" // for BrickPi3
#include <iostream>      // for cin and cout
#include <unistd.h>     // for usleep
#include <signal.h>     // for catching exit signals

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

uint16_t MinLight;
uint16_t MinColor;
uint16_t MaxLight;
uint16_t MaxColor;
sensor_light_t mylight;
sensor_color_t mycolor;

int16_t getlight(){
  BP.get_sensor(PORT_3, mylight);
  int16_t val = mylight.reflected;
  if (val < MinLight) val = MinLight;
  if (val > MaxLight) val = MaxLight;
  return (100-(100*(val - MinLight))/(MaxLight - MinLight));
}

int16_t getcolor(){
  BP.get_sensor(PORT_1, mycolor);
  uint16_t val = mycolor.reflected_red;
  if (val < MinColor) val = MinColor;
  if (val > MaxColor) val = MaxColor;
  return (100*(val - MinColor))/(MaxColor - MinColor);
}

void turn_left(void){
	BP.set_motor_power(PORT_B, 0);
	BP.set_motor_power(PORT_C, 0);
	BP.set_motor_position_relative(PORT_B, -500);
	BP.set_motor_position_relative(PORT_C, 500);
	sleep(1);
}

void turn_right(void){
	BP.set_motor_power(PORT_B, 0);
	BP.set_motor_power(PORT_C, 0);
	BP.set_motor_position_relative(PORT_B, 550);
	BP.set_motor_position_relative(PORT_C, -550);
	sleep(1);	
}

void forward(int time){
	BP.set_motor_power(PORT_B, 50);
	BP.set_motor_power(PORT_C, 50);
	sleep(time);
}

void move_aside(void){
	turn_right();
	forward(1);
	turn_left();
}

void dodge(sensor_ultrasonic_t Ultrasonic2){
	int done = 0;
	move_aside();
	while(done == 0){
    BP.set_motor_power(PORT_B, 50);
	  BP.set_motor_power(PORT_C, 50);
    for(int i = 0; i < 50; i++){
      if(getlight() > 90){
        done++;
        break;
      }
    }
		turn_left();
		if(BP.get_sensor(PORT_2, Ultrasonic2) == 0){
			if(Ultrasonic2.cm < 30){
				turn_right();
			}
			else{
				done++;
			}
 		}
	}
}

int main(){
  signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C

  BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.

  BP.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);
  BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
  BP.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_RED);
  
  sensor_ultrasonic_t Ultrasonic2;
  
  string regel;
  cout << "plaats sensor recht boven de lijn (zwart) en voer in a gevolgd door enter" << endl;
  cin >> regel;
  BP.get_sensor(PORT_3, mylight);
  BP.get_sensor(PORT_1, mycolor);
  MaxLight = mylight.reflected;
  MinColor = mycolor.reflected_red;
  cout << "MaxLight =" << MaxLight << endl;
  cout << "MinColor =" << MinColor << endl;
  cout << "plaats sensor recht boven de lijn (wit) en voer in a gevolgd door enter" << endl;
  cin >> regel;
  BP.get_sensor(PORT_3, mylight);
  BP.get_sensor(PORT_1, mycolor);
  MinLight = mylight.reflected;
  MaxColor = mycolor.reflected_red;
  cout << "MinLight =" << MinLight << endl;
  cout << "MaxColor =" << MaxColor << endl;
  
  int16_t lightval1;
  int16_t lightval2;
  int16_t power = 20;
  
while(true){ 
lightval1 = getlight();
lightval2 = getcolor();
 cout << lightval1 << "\n"; cout << lightval2 << "\n"; 
BP.set_motor_dps(PORT_B, 240); 
BP.set_motor_dps(PORT_C, 240); 
if((lightval1 <= 50) && (lightval2 <= 50)){
BP.set_motor_dps(PORT_B, 240); BP.set_motor_dps(PORT_C, 240);    
usleep(10000);
}
if((lightval1 <= 50) && (lightval2 <= 50)){
BP.set_motor_dps(PORT_B, 0);    BP.set_motor_dps(PORT_C, 0);
}
if (lightval1 <= 50){ 
BP.set_motor_dps(PORT_B, 300); BP.set_motor_dps(PORT_C, 0); 
}
if (lightval2 <= 50){
BP.set_motor_dps(PORT_B, 0); BP.set_motor_dps(PORT_C, 300); 
}
if (lightval1 <= 60){
BP.set_motor_dps(PORT_B, 240); BP.set_motor_dps(PORT_C, 0);
} 
if (lightval2 <= 60) { 
BP.set_motor_dps(PORT_B, 0); BP.set_motor_dps(PORT_C, 240);    
} 
if(lightval1 > 70 && lightval2 > 70){
BP.set_motor_dps(PORT_B, 200);    BP.set_motor_dps(PORT_C, 50);
}
usleep(100000);
}
}
  
  
 void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    BP.set_sensor_type(PORT_1, SENSOR_TYPE_NONE);  //Doesn't work, sorry.
    exit(-2);
  }
}
