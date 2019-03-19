/*
Project name: Arduino PID controller for EV3 using I2C protocol
Author: Ariel Lima Andrade
Email: lima.ariel97@gmail.com
Date: February 20th, 2019
© 2019 - Ariel Lima Andrade - All Rights Reserved
*/ 

#include <PID_v1.h>
/*
***************************************************************
* Arduino PID Library - Version 1.2.1
* by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
*
* This Library is licensed under the MIT License
***************************************************************
*/

#include <Wire.h>
#include <LiquidCrystal.h>

//-----------I2C PARAMETERS-------------
#define I2C_ADDRESS 0x04 //Define I2C address.
bool I2C_connected = 0; //Check if I2C is connect.
//--------------------------------------

//-----------PID PARAMETERS-------------
double KP = 0.0;   //Proportional constant
double KI = 0.0;       //Integral constant
double KD = 0.0;      //Derivative constant

double Setpoint = 0; //Reference position. PID controls the motor to reach this position.
double Input;        //Motor encoder feedback. 
double Output;       //Motor power.

PID myPID(&Input, &Output, &Setpoint, KP, KI, KD, DIRECT); //PID library. It sets the Output variable based on the error (Setpoint - Input);
//--------------------------------------

//-----------LCD PARAMETERS-------------
LiquidCrystal LCD(12, 11, 5, 4, 3, 2); //Display LCD library. 
//--------------------------------------

int encoder_feedback = 0; //Motor encoder feedback.
bool flag = 0; //Flag to verify whether the constants were defined or not.
int dig[6] = {}; //Digits on LCD screen.
byte b1, b2; //Buttons status.

float DTF(int *v); //Digits on LCD to float number.
void set_const(int *skip); //Setting KP, KI and KD, input by the user.
void update_const(int n); //Update the screen with the actual constant value.
void check_b1(); //Check if button 1 was pressed.
void check_b2(); //Check if button 2 was pressed.

void setup() 
{
    LCD.begin(16, 2);                 //Setting up the LCD 16x2.
    Wire.begin(I2C_ADDRESS);          //Defining Arduino I2C address.
    Wire.onReceive(readData);         //When receive data from master, the function readData runs.
    Wire.onRequest(writeData);        //When the master requests data, the function writeData runs.
   
    byte i = 0;                      //Set starting time, in seconds.
    for(i; i > 0; i--)
    {
        LCD.setCursor(0, 0);
        LCD.print(" PID controller ");
        LCD.setCursor(0, 1);
        LCD.print(" Starting in");
        LCD.setCursor(13,1);
        LCD.print(i);
        delay(1000);
        LCD.clear();
    }
    
    if(!I2C_connected)                      //Checking if I2C is connected.
    {
        while(I2C_connected == false)
        {
            LCD.setCursor(0, 0);
            LCD.print("I2C disconnected");
            LCD.setCursor(0, 1);
            LCD.print("   Waiting...   ");
        }
        LCD.clear();
        I2C_connected = true;
    }
    else
        I2C_connected = true;
       
    Serial.begin(9600);               //Starting Serial Monitor.
    myPID.SetMode(AUTOMATIC);         //Starting PID control.
    myPID.SetOutputLimits(-100, 100); //Setting limits to PID Output (from -100 to 100, because that's the limit in EV3).
}


void loop() {
    set_const(&flag); //Setting constants.
    myPID.SetTunings(KP, KI, KD, P_ON_E); //Applying the new constants to PID algorithm.
    Input = encoder_feedback; //Assigning motor encoder feedback to PID Input.
    myPID.Compute(); //Computing Output based on the error.
    

    //Shows the position and the output on the screen.
    Serial.println("Running");
    LCD.setCursor(0, 0);
    LCD.print("Power: ");
    LCD.setCursor(11, 0);
    LCD.print(Output);
    LCD.setCursor(0, 1);
    LCD.print("Position: ");
    LCD.setCursor(11, 1);
    LCD.print((float)encoder_feedback);
}

void readData(int byteCount) //Receive data from EV3
{
    while(Wire.available()) 
    {
        I2C_connected = true;

        //Receive 8 bits (Read 3 bits and discard 5 bits).
        int encoder_feedback_raw = Wire.read(); //Encoder feedback in degrees. 
        b1 = Wire.read(); //Button 1 status
        b2 = Wire.read(); //Button 2 status
        for (int i = 0; i < 5; i++)
            Wire.read();
        
        //Adjustment
        if (encoder_feedback_raw > 127) 
            encoder_feedback = encoder_feedback_raw - 256;
        else 
            encoder_feedback = encoder_feedback_raw;
    }
}

void writeData() //Send data to EV3
{
    I2C_connected = true;
    Wire.write((int)Output);
}



void check_b1()
{
    while(!b1) Serial.print(" ");
    while(b1) Serial.print(" ");
}



void check_b2()
{
    while(!b2) Serial.print(" ");
    while(b2) Serial.print(" ");
}



float DTF(int *v) //This function takes the digits selected by the user and transforms it in a float number.
{
    float out = 0.0;
    for(int i = 0; i < 6; i++)
    {
        out += v[i] * pow(10, 1-i);
    }
    return out;
}



void update_const(int n)
{
    LCD.setCursor(6,1);
    if(dig[n] == 10)
        dig[n] = 0;

    for(int j = 0; j < 6; j++)
    {
        if(j != 2)
        {
            if(j == 0 && dig[j] == 0)
                LCD.print(' ');
            else
                LCD.print(dig[j]);
        }
        else
        {
            LCD.print('.');
            LCD.print(dig[j]); 
     }
}



void set_const(bool *skip)
{
    if (!*skip)
    {
        LCD.clear();
        LCD.setCursor(2,0);
        LCD.print("SET KP,KI,KD");
        LCD.setCursor(5,1);
        LCD.print("VALUES");
        check_b1();
        LCD.clear();       
        LCD.setCursor(2,0);
       
        LCD.print("PROPORTIONAL");
        LCD.setCursor(3,1);
        LCD.print("KP=");
        update_const(0);
        
        for(int i = 0; i < 6; i++)
        {
            while(!b2)
            {
                 
                if (i < 2)
                    LCD.setCursor(6+i,1);
                else
                    LCD.setCursor(6+i+1,1);

                LCD.cursor();

                if(b1)
                {
                    while(b1) Serial.print(" ");;
                    dig[i]++;
                    update_const(i);
                } 
                else
                    Serial.print(" ");       
            }

            while(b2) Serial.print(" ");
         
        }      
        KP = DTF(dig);
        for(int k = 0; k < 6; k++) dig[k] = 0;
        LCD.clear();
         
        LCD.setCursor(4,0);
        LCD.print("INTEGRAL");
        LCD.setCursor(3,1);
        LCD.print("KI=");
        update_const(0);
        
        for(int i = 0; i < 6; i++)
        {
            while(!b2)
            {
                 
                if (i < 2)
                    LCD.setCursor(6+i,1);
                else
                    LCD.setCursor(6+i+1,1);

                LCD.cursor();

                if(b1)
                {
                    while(b1) Serial.print(" ");;
                    dig[i]++;
                    update_const(i);
                } 
                else
                    Serial.print(" ");       
            }

            while(b2) Serial.print(" ");
         
        }
        KI = DTF(dig);
        for(int k = 0; k < 6; k++) dig[k] = 0;
        LCD.clear();  
          
        LCD.setCursor(3,0);
        LCD.print("DERIVATIVE");
        LCD.setCursor(3,1);
        LCD.print("KD=");
        update_const(0);
         
        for(int i = 0; i < 6; i++)
        {
            while(!b2)
            {
                 
                if (i < 2)
                    LCD.setCursor(6+i,1);
                else
                    LCD.setCursor(6+i+1,1);

                LCD.cursor();

                if(b1)
                {
                    while(b1) Serial.print(" ");;
                    dig[i]++;
                    update_const(i);
                } 
                else
                    Serial.print(" ");       
            }

            while(b2) Serial.print(" ");
         
        }      
        KD = DTF(dig);
        for(int k = 0; k < 6; k++) dig[k] = 0;
        LCD.clear();


        LCD.noCursor();      
        *skip = 1;
         
    }
    else
        ;
}
