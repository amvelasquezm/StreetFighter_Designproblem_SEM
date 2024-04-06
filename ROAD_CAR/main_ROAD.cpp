/*######################################################################
  //#  G0B1T: uC EXAMPLES. 2024.
  //######################################################################
  //# Copyright (C) 2024. F.E.Segura-Quijano (FES) fsegura@uniandes.edu.co
  //#
  //# This program is free software: you can redistribute it and/or modify
  //# it under the terms of the GNU General Public License as published by
  //# the Free Software Foundation, version 3 of the License.
  //#
  //# This program is distributed in the hope that it will be useful,
  //# but WITHOUT ANY WARRANTY; without even the implied warranty of
  //# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  //# GNU General Public License for more details.
  //#
  //# You should have received a copy of the GNU General Public License
  //# along with this program.  If not, see <http://www.gnu.org/licenses/>
  //####################################################################*/

//=======================================================
//  LIBRARY Definition
//=======================================================
#include <Arduino.h>

#define REALMATRIX // Variable to use real matrix. Comment to not use it.

#ifdef REALMATRIX
#include "LedControl.h"
/* Pin definition for MAX72XX.
 ARDUINO pin 12 is connected to the DataIn  - In ESP32 pin 23
 ARDUINO pin 11 is connected to the CLK     - In ESP32 pin 18
 ARDUINO pin 10 is connected to LOAD        - In ES32 pin 5
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(23,18,5,1);
#endif

//=======================================================
//  IF PROBLEMS
//=======================================================
// In LedControl.h  change the following line:
// #include <avr/pgmspace.h>
// by 
/* #if (defined(_AVR_))
 #include <avr\pgmspace.h>
 #else
 #include <pgmspace.h>
 #endif
*/

//=======================================================
//  REGISTERS (VARIABLES) to control the game
//=======================================================
/* Registers to background cars.*/
byte RegBACKGTYPE_dataRANDOM;
byte RegBACKGTYPE_dataZEROS = B00000000;

/* Time delays. */
unsigned long delaytime = 6000;

/* Global Variables */
int i = 0;
int count = 0;
int points = 0;

/* States ans signals to change state*/
enum State_enum {STATERESET, STATESTART, STATECLEAR, STATECHECK, STATELEFT, STATERIGTH, STATELOST};
uint8_t state = STATERESET;

enum Keys_enum {RESET_KEY, START_KEY, LEFT_KEY, RIGHT_KEY, NO_KEY};
uint8_t keys = RESET_KEY;

enum Status_enum {LOST, CONTINUE};
uint8_t Status = CONTINUE;

/* Key to control game by serial input. */
  int incomingByte;

/* Pointer and Matrix to Control Driver. */
  byte RegMatrix[8];
  byte *pointerRegMatrix;

/* Pointer and Register to control bottom car. */
  byte RegCar[1];
  byte *pointerRegCar;

/* Pointer and Register (Variable) to move bottom car */
  byte ShiftDir[1];
  byte *pointerShiftDir;

//=======================================================
//  SETUP Arduino function
//=======================================================
/* Setup function initialization */
void setup()
{
#ifdef REALMATRIX
  /* The MAX72XX is in power-saving mode on startup, we have to do a wakeup call. */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values. */
  lc.setIntensity(0, 8);
  /* Clear the display. */
  lc.clearDisplay(0);
#endif
  /* Serial port initialization. */
  Serial.begin(9600);

  /* Pointer to use Matrix between functions. */
  pointerRegMatrix = &RegMatrix[0];

  /* Pointer to use VectorCar between functions. */
  pointerRegCar = &RegCar[0];

  /* Pointer to use shift dir between functions */
  pointerShiftDir = &ShiftDir[0];
}

//=======================================================
//  FUNCTION: writeResetMatrix
//=======================================================
/* Data matrix when reset*/
void writeResetMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to reset matrix */
  pointerRegMatrix[7] = B11111111;
  pointerRegMatrix[6] = B11111111;
  pointerRegMatrix[5] = B11111111;
  pointerRegMatrix[4] = B11111111;
  pointerRegMatrix[3] = B11111111;
  pointerRegMatrix[2] = B11111111;
  pointerRegMatrix[1] = B11111111;
  pointerRegMatrix[0] = B11111111;
  /* Here is the data to reset bottomCar */
  pointerRegCar[0] = B00000000;
}
//=======================================================
//  FUNCTION: writeStartMatrix
//=======================================================
void writeStartMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to start matrix */
  pointerRegMatrix[7] = B01111110;
  pointerRegMatrix[6] = B10000001;
  pointerRegMatrix[5] = B10011001;
  pointerRegMatrix[4] = B10000001;
  pointerRegMatrix[3] = B10100101;
  pointerRegMatrix[2] = B10011001;
  pointerRegMatrix[1] = B10000001;
  pointerRegMatrix[0] = B01111110;
/* Here is the data to start bottomCar */
  pointerRegCar[0] = B00000000;
}
//=======================================================
//  FUNCTION: writeClearMatrix
//=======================================================
void writeClearMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to clear matrix */
  pointerRegMatrix[7] = B00000000;
  pointerRegMatrix[6] = B00000000;
  pointerRegMatrix[5] = B00000000;
  pointerRegMatrix[4] = B00000000;
  pointerRegMatrix[3] = B00000000;
  pointerRegMatrix[2] = B00000000;
  pointerRegMatrix[1] = B00000000;
  pointerRegMatrix[0] = B00000000;
  /* Here is the data to clear bottomCar */
  pointerRegCar[0] = B00010000;
}
//=======================================================
//  FUNCTION: writeLostMatrix
//=======================================================
void writeLostMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */

  /* Here is the data to lost matrix */
  pointerRegMatrix[7] = B01111110;
  pointerRegMatrix[6] = B10000001;
  pointerRegMatrix[5] = B10011001;
  pointerRegMatrix[4] = B10000001;
  pointerRegMatrix[3] = B10011001;
  pointerRegMatrix[2] = B10100101;
  pointerRegMatrix[1] = B10000001;
  pointerRegMatrix[0] = B01111110;
  /* Here is the data to lost matrix */
  pointerRegCar[0] = B00000000;
  count = 0;
}
//=======================================================
//  FUNCTION: writeGoCarsMatrix
//=======================================================


int getRandomNumber(int *numbers, int num_numbers) {
    // Inicializar el generador de números aleatorios
    srand(time(NULL));

    // Seleccionar un número aleatorio de la lista
    int random_index = rand() % num_numbers;
    return numbers[random_index];
}

void writeGoCarsMatrix(byte *pointerRegMatrix)
{
  /* Global variables. */
  int m;
  int numbers[] = {2, 3, 4, 5, 6, 8, 9, 10, 12, 16, 17, 18, 20, 24, 32, 33, 34, 36, 40, 48};
  int num_numbers = sizeof(numbers) / sizeof(numbers[0]);

  int selected_number = getRandomNumber(numbers, num_numbers);

  i = i + 1;
  /* Here is the data to start matrix */
  RegBACKGTYPE_dataRANDOM = selected_number; 

  for (m = 0; m < 7; m++)
  {
    pointerRegMatrix[m] = pointerRegMatrix[m + 1];
  }
  count = count + 1;

  if (i % 2 == 0)
    pointerRegMatrix[7] = RegBACKGTYPE_dataRANDOM;
  else
    pointerRegMatrix[7] = RegBACKGTYPE_dataZEROS;

  Serial.print("I received: ");
  Serial.println(count);

  if (count == 20){
      points = 10;
      count = 0;
  }

}

void writeGoCarsMatrix2(byte *pointerRegMatrix)
{
  /* Global variables. */

  int m;
  int numbers[] = {2, 67, 4, 5, 6, 50, 9, 70, 12, 16, 17, 18, 20, 24, 32, 33, 34, 36, 40, 48, 7, 11, 13, 14, 19, 21, 22, 25, 26, 30};
  int num_numbers = sizeof(numbers) / sizeof(numbers[0]);

  int selected_number = getRandomNumber(numbers, num_numbers);

  i = i + 1;
  /* Here is the data to start matrix */
  RegBACKGTYPE_dataRANDOM = selected_number; 


  for (m = 0; m < 7; m++)
  {
    pointerRegMatrix[m] = pointerRegMatrix[m + 1];
  }
  count = count + 1;

  if (i % 2 == 0)
    pointerRegMatrix[7] = RegBACKGTYPE_dataRANDOM;
  else
    pointerRegMatrix[7] = RegBACKGTYPE_dataZEROS;

  Serial.print("I received: ");
  Serial.println(count);

  if (count == 25){
      points = 25;
      count = 0;
  }
}

void writeGoCarsMatrix3(byte *pointerRegMatrix){
  /* Global variables. */
  int m;
  RegBACKGTYPE_dataRANDOM = random(1, 255);
  i = i + 1;
  /* Here is the data to start matrix */
  for (m = 0; m < 7; m++)
  {
    pointerRegMatrix[m] = pointerRegMatrix[m + 1];
  }
  count = count + 1;

  if (i % 2 == 0)
    pointerRegMatrix[7] = RegBACKGTYPE_dataRANDOM;
  else
    pointerRegMatrix[7] = RegBACKGTYPE_dataZEROS;

  Serial.print("I received: ");
  Serial.println(count);

  if (count == 30){
      points = 45;
      count = 0;
  }
}

//=======================================================
//  FUNCTION: writeCarBase
//=======================================================
void writeCarBase(byte *pointerRegCar, byte *pointerShiftDir)
{
  /* Global variables. */
  int m;
  
  /* Here is the data to start matrix */
  if (pointerShiftDir[0] == B00000001)
  {
    if (pointerRegCar[0] == B00000001)
      pointerRegCar[0] = pointerRegCar[0];
    else
      pointerRegCar[0] = pointerRegCar[0] >> 1;
  }
  else if (pointerShiftDir[0] == B00000010)
  {
    if (pointerRegCar[0] == B10000000)
      pointerRegCar[0] = pointerRegCar[0];
    else
      pointerRegCar[0] = pointerRegCar[0] << 1;
  }
  else
    pointerRegCar[0] = pointerRegCar[0];
}
//=======================================================
//  FUNCTION: checkLostMatrix (leds and console)
//=======================================================
void checkLostMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */
  byte check1, check2;

  check1 = pointerRegCar[0] ^ pointerRegMatrix[0];
  check2 = pointerRegCar[0] | pointerRegMatrix[0];

  if (pointerRegCar[0] == pointerRegMatrix[0])
    Status = LOST;
  else if (check1 != check2)
    Status = LOST;
  else
    Status = CONTINUE;
}
//=======================================================
//  FUNCTION: printBits (by console all bits)
//=======================================================
void printBits(byte myByte)
{
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask  & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}
//=======================================================
//  FUNCTION: PrintMatrix (Console)
//=======================================================
void PrintMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */
  int m;

  for (m = 7; m >= 1; m--)
  {
    printBits(pointerRegMatrix[m]);
    Serial.println();
  }
  printBits(pointerRegMatrix[0] | pointerRegCar[0]);
  Serial.println();
}
//=======================================================
//  FUNCTION: PrintALLMatrix (leds and console)
//=======================================================
void PrintALLMatrix(byte *pointerRegMatrix, byte *pointerRegCar)
{
  /* Global variables. */
  int m;

#ifdef REALMATRIX
  /* Display data one by one in matrix. */
  for (m = 7; m >= 1; m--)
  {
    lc.setRow(0, m, pointerRegMatrix[m]);
  }
  lc.setRow(0, m, (pointerRegMatrix[0] | pointerRegCar[0]));
#endif
  /* Display data one by one in console. */
  Serial.println("########");
  Serial.println("########");
  Serial.println("########");
  Serial.println("########");
  PrintMatrix(pointerRegMatrix, pointerRegCar);
  Serial.println("########");
}


//=======================================================
//  FUNCTION: read_KEY
//=======================================================
byte read_KEY(void)
{
  if (Serial.available
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  () > 0)
  {
    incomingByte = Serial.read();
    
    if (points == 25){
      delay(1/10);
    }else{
      delay(1/4);
    }
    //    Serial.print("I received: ");
    //    Serial.println(incomingByte, DEC);
  }
  switch (incomingByte)
  {
    case 'R':
      keys = RESET_KEY;
      break;
    case 'S':
      keys = START_KEY;
      break;
    case 'A':
      keys = LEFT_KEY;
      break;
    case 'D':
      keys = RIGHT_KEY;
      break;
    default:
      keys = NO_KEY;
      break;
  }
return keys;
}
//=======================================================
//  FUNCTION: state_machine_run
//=======================================================
void state_machine_run(byte *pointerRegMatrix, byte *pointerRegCar, byte *pointerShiftDir)
{
  /* Global variables. */
  int count = 0;

  PrintALLMatrix(pointerRegMatrix, pointerRegCar);

  switch (state)
  {
    case STATERESET:
      writeResetMatrix(pointerRegMatrix,pointerRegCar);
      delay(delaytime);
      if (keys == RESET_KEY)
        state = STATERESET;
      else if (keys == START_KEY)
        state = STATESTART;
      break;

    case STATESTART:
      writeStartMatrix(pointerRegMatrix,pointerRegCar);
      delay(delaytime);
      state = STATECLEAR;
      count = 0;
      break;

    case STATECLEAR:
      writeClearMatrix(pointerRegMatrix,pointerRegCar);
      delay(delaytime);
      state = STATECHECK;
      break;

    case STATECHECK:
      Serial.print("count:");
      Serial.println(count);
      pointerShiftDir[0] = B00000000;
      writeCarBase(pointerRegCar, pointerShiftDir);

      if (points == 0){
         
         Serial.print("NVL1");
         writeGoCarsMatrix(pointerRegMatrix);
      }
      else if (points == 10){
        delaytime = 7000/3;
        Serial.print("NVL2");
        writeGoCarsMatrix2(pointerRegMatrix);

      }
      else if (points == 25){
        delaytime = 7000/9;
        Serial.print("NVL3");
        writeGoCarsMatrix3(pointerRegMatrix);
      }
      else if (points == 45){
        
        pointerRegMatrix[7] = B00000000;
        pointerRegMatrix[6] = B01100110;
        pointerRegMatrix[5] = B10011001;
        pointerRegMatrix[4] = B10000001;
        pointerRegMatrix[3] = B10000001;
        pointerRegMatrix[2] = B01000010;
        pointerRegMatrix[1] = B00100100;
        pointerRegMatrix[0] = B00011000;
        
      }

      delay(delaytime);
      checkLostMatrix(pointerRegMatrix, pointerRegCar);

      if (Status == LOST){

        state = STATELOST;
        count = 0;
        points = 0;
        }
      else if (keys == RESET_KEY)
        state = STATERESET;
      else if (keys == LEFT_KEY)
        state = STATELEFT;
      else if (keys == RIGHT_KEY)
        state = STATERIGTH;
      else{
        state = STATECHECK;
      }
      break;

    case STATELEFT:
      pointerShiftDir[0] = B00000001;
      writeCarBase(pointerRegCar, pointerShiftDir);
      state = STATECHECK;
      break;

    case STATERIGTH:
      pointerShiftDir[0] = B00000010;
      writeCarBase(pointerRegCar, pointerShiftDir);
      state = STATECHECK;
      break;

    case STATELOST:
      writeLostMatrix(pointerRegMatrix,pointerRegCar);
      count = 0;

      if (keys == START_KEY)
        state = STATESTART;
      else
        state = STATELOST;
      break;

    default:
      state = STATERESET;
      break;
  }
}

//=======================================================
//  FUNCTION: Arduino loop
//=======================================================
void loop()
{
  
  read_KEY();
  state_machine_run(pointerRegMatrix,pointerRegCar,pointerShiftDir);
  delay(1);
}