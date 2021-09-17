/*
  A simple Pong game. Thank you KonstantinRU [arduino.cc forums] for the base code! 
  The most important changes were due to the fact that KonstantinRU used a potentiometer to control the paddle position whereas pocKonso uses switches.
  I have left KonstantinRU's code for the potentiometer control but, of course, commented it out.
  I believe KonstatinRU also used an SPI display; I have adapted the code for I2C. 
  I have also added comments to explain various values and parameters. 
  Note: pitches.h not necessary- tone frequency value can be input directly into soundBounce() and soundPoint()
 */

//#include <SPI.h> // Commented out because using I2C ; if using SPI display uncomment this line
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#include "pitches.h"

//const int SW_pin = 4; // digital pin connected to switch output
//const int Y_pin = 1; // analog pin connected to Y output
#define UP_BUTTON 9      // Added for pocKonso
#define DOWN_BUTTON 6     // Added for pocKonso

const unsigned long PADDLE_RATE = 33; // paddle movement speed higher is slower
const unsigned long BALL_RATE = 5; // ball movement speed higher value is slower
const uint8_t PADDLE_HEIGHT = 12; //12
int playerScore = 0;
int aiScore = 0;
int maxScore = 8;
int BEEPER = 0; // 12  Pin #
bool resetBall = false; // false
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
//#define RESET_BUTTON 3          // Not necessary for pocKonso
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void drawCourt();
void drawScore();

uint8_t ball_x = 64, ball_y = 32;
uint8_t ball_dir_x = 1, ball_dir_y = 1;
unsigned long ball_update;

unsigned long paddle_update;
const uint8_t CPU_X = 22;
uint8_t cpu_y = 26;

const uint8_t PLAYER_X = 105;
uint8_t player_y = 6;

void setup() {  

 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
display.setRotation(2); // ANGUS: This is (2) value is necessary because display mounted "upside-down"  
    display.display();
    unsigned long start = millis();
    pinMode(BEEPER, OUTPUT);
    pinMode(UP_BUTTON, INPUT);
    pinMode(DOWN_BUTTON, INPUT);
    digitalWrite(UP_BUTTON,1);
    digitalWrite(DOWN_BUTTON,1);
    //pinMode(RESET_BUTTON, INPUT_PULLUP); // commented out for pocKonso; 
  //  digitalWrite(SW_pin, HIGH);         //  same 
    display.clearDisplay();
    drawCourt();
    drawScore();  
    while(millis() - start < 2000);

    display.display();

    ball_update = millis();
    paddle_update = ball_update;
}

void loop() {
    bool update = false;
    unsigned long time = millis();

    static bool up_state = false;
    static bool down_state = false;
    
up_state |= (digitalRead(UP_BUTTON) == LOW);          //ANGUS: code added for switch control
    down_state |= (digitalRead(DOWN_BUTTON) == LOW); // ANGUS: same


    if(resetBall)
    {
     if(playerScore == maxScore || aiScore == maxScore)
            {
              gameOver();
            }
      else{      
      display.fillScreen(BLACK);
      drawScore();
      drawCourt();       
      ball_x = random(45,50);   // ANGUS : these two lines randomize the start position of the ball
      ball_y = random(23,33);     //ANGUS
      do
      {
      ball_dir_x = random(-1,2);    // ANGUS: these next lines randomize the start direction the ball... this needs to be tweaked 
      }while(ball_dir_x==0);        //  at the moment the ball too often is impossible for the AI to return

       do
      {
      ball_dir_y = random(-1,2);
      }while(ball_dir_y==0);
      
      
      resetBall=false;
      }
    }

    
    //up_state |= (digitalRead(UP_BUTTON) == LOW);  // KonstantinRU for potentiometer - commented out
   // down_state |= (digitalRead(DOWN_BUTTON) == LOW); // same
   
    if(time > ball_update) {
        uint8_t new_x = ball_x + ball_dir_x;
        uint8_t new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if(new_x == 0 || new_x == 127) {
         
          if(new_x == 0){
            playerScore+=1;
            display.fillScreen(BLACK);
            soundPoint();
            resetBall = true;
            
          }
          else if(new_x == 127){
            aiScore+=1;
            display.fillScreen(BLACK);
            soundPoint();
            resetBall = true;
          }       
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the horizontal walls.
        if(new_y == 0 || new_y == 63) {
            soundBounce();
            ball_dir_y = -ball_dir_y;
            new_y += ball_dir_y + ball_dir_y;
        }

        // Check if we hit the CPU paddle
        if(new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT) {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the player paddle
        if(new_x == PLAYER_X
           && new_y >= player_y
           && new_y <= player_y + PADDLE_HEIGHT)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        display.drawPixel(ball_x, ball_y, BLACK);
        display.drawPixel(new_x, new_y, WHITE);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        update = true;
    }

    if(time > paddle_update) {
        paddle_update += PADDLE_RATE;

        // CPU paddle
        display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, BLACK);
        const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
        if(cpu_y + half_paddle > ball_y) {
            cpu_y -= 1;
        }
        if(cpu_y + half_paddle < ball_y) {
            cpu_y += 1;
        }
        if(cpu_y < 1) cpu_y = 1;
        if(cpu_y + PADDLE_HEIGHT > 63) cpu_y = 63 - PADDLE_HEIGHT;
        display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT, WHITE);

        // Player paddle
    /*    display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK); //Entire section is KonstantinRU control of potentiometer
        if(analogRead(Y_pin) < 480) {
            player_y -= 1;
        }
        if(analogRead(Y_pin) > 510) {
            player_y += 1;
        }
        up_state = down_state = false;
        if(player_y < 1) player_y = 1;
        if(player_y + PADDLE_HEIGHT > 63) player_y = 63 - PADDLE_HEIGHT;
        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);
    } update = true;
    */
    display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK); //new
//const uint8_t half_paddle = PADDLE_HEIGHT >> 1; //new
    
if(up_state) {
            player_y -= 1;
        }
        if(down_state) {
            player_y += 1;
        }
        up_state = down_state = false;
        if(player_y < 1) player_y = 1;
        if(player_y + PADDLE_HEIGHT > 63) player_y = 63 - PADDLE_HEIGHT;
        display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);

  }  
        update = true;
// }
   if(update){
        drawScore();
        display.display();
        if (digitalRead(UP_BUTTON) == 0) //(SW_pin)
        if (digitalRead(DOWN_BUTTON) == 0) // New line Angus
        {
         gameOver();
        }
        }
}

void drawCourt() {
    display.drawRect(0, 0, 128, 64, WHITE);
}
void drawScore() {
  // draw AI and player scores
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(45, 0);
  display.println(aiScore);
  display.setCursor(75, 0);
  display.println(playerScore);
}

void gameOver(){ 
  display.fillScreen(BLACK);
  if(playerScore>aiScore)
  {
    display.setCursor(20,4);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.print("Good      game!");   // ANGUS: You can change the text for when the AI loses
  }else
  {
    display.setCursor(45,4);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.print("You    played     well !");     // ANGUS: Change text for player losing
  }
 delay(200);
 display.display();
 delay(2000);
 aiScore = playerScore = 0;
  
unsigned long start = millis();
while(millis() - start < 2000);
ball_update = millis();    
paddle_update = ball_update;
resetBall=true;
}

void soundBounce() 
{
  tone(BEEPER, 500, 50);   // ANGUS: 500 is the tone's frequency and 50 is the tones duration. Change to taste. 
}
void soundPoint() 
{
  tone(BEEPER, 250, 50); // ANGUS: same
}
