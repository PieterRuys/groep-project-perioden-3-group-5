#include <iomanip>
#include "BrickPi3.h"
#include <iostream> 
#include <unistd.h> 
#include <signal.h> 
using namespace std;

void exit_signal_handler(int signo);

uint16_t MinLight;
uint16_t MinColor;
uint16_t MaxLight;
uint16_t MaxColor;
sensor_light_t mylight;
sensor_color_t mycolor;

BrickPi3 BP;

const int board_width = 5;
const int board_heigth = 5;

char board[board_width][board_heigth];

struct Pos {
    int x;
    int y;
};

struct Pos rob_pos;
int rob_dir;

int16_t getlight(){ // An code for finding the percentage of light coming in
  BP.get_sensor(PORT_3, mylight);
  int16_t val = mylight.reflected;
  if (val < MinLight) val = MinLight;
  if (val > MaxLight) val = MaxLight;
  return 100-((100*(val - MinLight))/(MaxLight - MinLight));
}

int16_t getcolor(){ // An code for finding the percentage of light coming in measured in red
  BP.get_sensor(PORT_1, mycolor);
  uint16_t val = mycolor.reflected_red;
  if (val < MinColor) val = MinColor;
  if (val > MaxColor) val = MaxColor;
  return (100*(val - MinColor))/(MaxColor - MinColor);
}

void robot_turn_left(void){ // An code to turn left
	BP.set_motor_power(PORT_B, 0);
	BP.set_motor_power(PORT_C, 0);
	BP.set_motor_position_relative(PORT_B, -560);
	BP.set_motor_position_relative(PORT_C, 560);
	sleep(1);
}

void robot_turn_right(void){ // An code to turn right
	BP.set_motor_power(PORT_B, 0);
	BP.set_motor_power(PORT_C, 0);
	BP.set_motor_position_relative(PORT_B, 560);
	BP.set_motor_position_relative(PORT_C, -560);
	sleep(1);
}

bool next_crosing_free(sensor_ultrasonic_t Ultrasonic2){ // An code to detect if there is an object in the way
 	if(BP.get_sensor(PORT_2, Ultrasonic2) == 0){
		if(Ultrasonic2.cm < 20){
			return true;
		}
		else{
			return false;
		}
  	}
}

void robot_forward_one_step(){ // Here the robot drives forward until it spots a black marking with both sensors
  while(true){
    int16_t colorval = getcolor();
    int16_t lightval = getlight();
    
    if(lightval <= 20 && colorval <= 20){
      //drive until you are on the crossing
      sleep(1);
      BP.set_motor_power(PORT_B, 0);
      BP.set_motor_power(PORT_C, 0);
      break;
    }
    else{
      	BP.set_motor_power(PORT_B, 30);
      	BP.set_motor_power(PORT_C, 30);
   }
    usleep(10000);
  }
}

void init_board() { // Here the board on wich the robot will rive gets made

    for ( int x = 0; x < board_width; x++ ) {
        for ( int y = 0; y < board_heigth; y++ ) {
            board[x][y] = '~';
        }
    }
}

bool pos_move_one_step(struct Pos &pos, int direction) {
// here the new possition gets calculated by checking the previous direction and adding a nummber on the x or y axis acordingly
    struct Pos new_pos = pos;

    switch ( direction) {
    case 0: // east
        new_pos.x++;
        break;
    case 1: // south
        new_pos.y++;
        break;
    case 2: // west
        new_pos.x--;
        break;
    case 3: // north
        new_pos.y--;
        break;
    }

    // Are we still on the board?
    if ( new_pos.x < 0 || new_pos.x >= board_width ) return false;
    if ( new_pos.y < 0 || new_pos.y >= board_heigth ) return false;

    // Is the position not already done?
    if ( board[new_pos.x][new_pos.y] != '~' ) return false;

    pos = new_pos;

    return true;
}

bool detect_obstacle_ahead(struct Pos rob_pos, int direction, sensor_ultrasonic_t Ultrasonic2){ // Here it checks if there is an obstacle and palces an X if there is
	if ( pos_move_one_step(rob_pos, direction) ) {
		BP.set_motor_power(PORT_B, 0);
        	BP.set_motor_power(PORT_C, 0);
		sleep(1);
		if(next_crosing_free(Ultrasonic2)) board[rob_pos.x][rob_pos.y] = 'X';
	}
	BP.set_motor_power(PORT_B, 30);
        BP.set_motor_power(PORT_C, 30);
}

bool run(struct Pos rob_pos, int direction, sensor_ultrasonic_t Ultrasonic2) { // This is the recursive code that walks the board

    board[rob_pos.x][rob_pos.y] = '*'; // Mark 'been here'

    if ( rob_pos.x == board_width - 1 && rob_pos.y == board_heigth - 1 ) {
        cout << "Hurray!" << endl;//the bot has reached its final desstination
        return true;
    }

    detect_obstacle_ahead(rob_pos, direction, Ultrasonic2);
	
    if ( pos_move_one_step(rob_pos, direction) ) {
	robot_forward_one_step();
        for ( int d = 0; d < 4; d++ ) {
	    int new_direction = (direction + d) % 4;//molo to make shure the direction is always 0, 1, 2 or 3
            if ( run(rob_pos, new_direction, Ultrasonic2) ) {//here the function is made recursive, this is done so the robot can check alloptions in a pace. if there is nowere to go it will go back to the previous space in that space's position.
                return true;
	    }
            if (d != 3) {
		robot_turn_right();//the last turn is skiped so we can turn back
                detect_obstacle_ahead(rob_pos, new_direction, Ultrasonic2);
            }
        }
        robot_turn_left();
        robot_forward_one_step();
        robot_turn_left();
        robot_turn_left();
        // we have now returnd to our origional possition
    }

    board[rob_pos.x][rob_pos.y] = 'X'; // this space is a dead end, so we mark it of

    return false;
}

int main() {
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
	cout << "klaar?" << endl;
	cin >> regel;

    init_board();
    struct Pos rob_pos = {0, 0};
	
    rob_pos.x = rob_pos.y = 0;
    rob_dir = 0;

    for ( int direction = 0; direction < 4 ; direction++ ) {
        if ( run(rob_pos, direction, Ultrasonic2) ) {
            break;
        }
	robot_turn_right();
    }

    return 0;
}

 void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    BP.set_sensor_type(PORT_1, SENSOR_TYPE_NONE);  //Doesn't work, sorry.
    exit(-2);
  }
}
