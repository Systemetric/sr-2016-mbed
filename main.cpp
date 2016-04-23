
#include "mbed.h"
#define WRISTS_0            0
#define WRISTS_n90          1
#define WRISTS_90           2
#define WRISTS_60           3
#define WRISTS_n120         4
#define WRISTS_n30          5
#define WRISTS_unknown1     6
#define WRISTS_unknown2     7

#define PLATTER_0           0    
#define PLATTER_90          1
#define PLATTER_180         2
#define PLATTER_270         3
#define PLATTER_CR          4
#define PLATTER_GRAB        128
#define PLATTER_RELEASE     0

#define LIFT_0              0
#define LIFT_42             1   
#define LIFT_120            2

#define ARMS_IN             0
#define ARMS_OUT            1

/*
These control the following commands '>'=recieve '<'=send:
>F[2B]<!<d   drive forwards  [distance in    mm]
>B[2B]<!<d   drive backwards [distance in    mm]

>L[2B]<!<d   turn left       [angle in       degrees]
>R[2B]<!<d   turn right      [angle in       degrees]

>A[1B]<!<d   move arms       [0: 0, 1: 42, 2: 120]
>H[1B]<!<d   move hands      [0: 0, 1: -90, 2: 90, 3:60, 4: -120, 5: -30, 6: U1, 7: U2]

>G[1B]<!     grab            
>g[1B]<!     release         

>T<!        turn Platter continuously
>0<!        turn Platter to position 0 (000deg)
>1<!        turn Platter to position 1 (090deg)
>2<!        turn Platter to position 2 (180deg)
>3<!        turn Platter to position 3 (270deg)
>P<!        suck
>p<!        stopSucking
*/


DigitalInOut motorStepPin(p22);
DigitalInOut leftMotorDirection(p23);//These have a different header instead of power-pulse-direction it is power-direction-pulse (does not affect code)
//DigitalOut enable(p21);
DigitalInOut rightMotorDirection(p21);//These have a different header instead of power-pulse-direction it is power-direction-pulse (does not affect code)
Serial odroid(USBTX, USBRX);
//DigitalOut PlatterCR(p20);
//DigitalOut Platter0(p19);
//DigitalOut Platter1(p18);
//DigitalOut Platter2(p17);
//DigitalOut Platter3(p16);
Serial wrists(p13, p14);  // tx, rx
Serial platter(p9, p10);  // tx, rx
Serial lift  (p28, p27);
DigitalOut arms (p12);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

#define oneDegree (2500.0/90.0)//was 2650.0-->2026
#define oneMM (2650.0/325.0)
#define stepTime 0.0003
//2650 steps gives 180 degrees and 650mm straight line
//2650 steps now gives 90 degrees and 325mm straight line

int currentPlatter = 0;
int currentSuck = 0;

void setDirection(bool left, bool right){
    if(!left){
         leftMotorDirection.output();
         leftMotorDirection = 0;
    }
    else{
        leftMotorDirection.input();    
    }
    if(right){
         rightMotorDirection.output();
         rightMotorDirection = 0;
    }
    else{
        rightMotorDirection.input();    
    }
}

void motorStep(bool a){
    if(a){
         motorStepPin.output();
         motorStepPin = 0;
    }
    else{
        motorStepPin.input();    
    }
}

void turnBy(float angle, bool direction)
{
    int stepsRemaining=(angle*oneDegree);
    setDirection(direction, !direction);
    while(stepsRemaining!=0) {
        motorStep(1);
        wait(stepTime);
        motorStep(0);
        wait(stepTime);
        stepsRemaining=stepsRemaining-1;
    }
}

void driveBy(float distance, bool direction)
{
    int stepsRemaining=(distance*oneMM);
    setDirection(direction, direction);
    while(stepsRemaining!=0) {
        motorStep(1);
        wait(stepTime);
        motorStep(0);
        wait(stepTime);
        stepsRemaining=stepsRemaining-1;
    }
}

void movePlatter(int mode)
{
    platter.putc(mode);
}

//true: grab
//false: release
void grab(bool mode)
{
    if(mode){
        arms = ARMS_IN;
    }
    else{
        arms = ARMS_OUT;    
    }  
}

void liftTo(int mode)
{
    lift.putc(mode);  
    switch(mode)
    {
        case 0:
            led1=0;
            led2=0;
            led3=1;
            led4=0;
            break;
        case 1:
            led1=0;
            led2=0;
            led3=0;
            led4=1;
            break;
        case 2:
            led1=0;
            led2=0;
            led3=1;
            led4=1;
            break;   
    }  
}

void wristsTo(int mode)
{
    wrists.putc(mode);    
}

int main() {
    wrists.baud(9600);
    platter.baud(9600);
    lift.baud(9600);
    
    wristsTo(2);
    wait(1);
    wristsTo(4);
    wait(1);
    wristsTo(0);
    wait(1);
    
    liftTo(2);
    
    movePlatter(0);
    
    //enable=1;
    int payload;
    while (1) 
    {
        if (odroid.getc()=='s') 
        {
            //found start character
            //odroid.putc ('!');
            switch (odroid.getc())
            {
                default:
                    //didn't get an expected command
                    odroid.putc('?');
                    break;
                case 'F':
                    led1=1;
                    led2=0;
                    led3=0;
                    led4=0;
                    payload = odroid.getc()<<8;
                    payload = payload + odroid.getc();
                    odroid.printf ("!");
                    driveBy(payload,1);
                    odroid.putc('d');
                    break;
                case 'B':
                    led1=1;
                    led2=0;
                    led3=0;
                    led4=1;
                    payload = odroid.getc()<<8;
                    payload = payload + odroid.getc();
                    odroid.printf ("!");
                    driveBy(payload,0);
                    odroid.putc('d');
                    break;
                case 'R':
                    led1=1;
                    led2=1;
                    led3=0;
                    led4=0;
                    payload = odroid.getc()<<8;
                    payload = payload + odroid.getc();
                    odroid.printf ("!");
                    turnBy(payload,1);
                    odroid.putc('d');
                    break;
                case 'L':
                    led1=1;
                    led2=1;
                    led3=0;
                    led4=1;
                    payload = odroid.getc()<<8;
                    payload = payload + odroid.getc();
                    odroid.printf ("!");
                    turnBy(payload,0);
                    odroid.putc('d');
                    break;
                case 'T':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_CR;
                    movePlatter(currentSuck | currentPlatter);
                    break;
                case '0':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_0;
                    movePlatter(currentSuck | currentPlatter);
                    break;
                case '1':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_90;
                    movePlatter(currentSuck | currentPlatter);
                    break;
                case '2':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_180;
                    movePlatter(currentSuck | currentPlatter);
                    break;
                case '3':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_270;
                    movePlatter(currentSuck | currentPlatter);
                    break;
                case 'P':
                    odroid.printf ("!");
                    currentSuck = PLATTER_GRAB;
                    movePlatter(currentSuck | currentPlatter);
                    break;
                case 'p':
                    odroid.printf ("!");
                    currentSuck = PLATTER_RELEASE;
                    movePlatter(currentSuck | currentPlatter);
                    break;
                case 'G':
                    odroid.printf("'");
                    grab(true);
                    break;
                case 'g':
                    odroid.printf("!");
                    grab(false);
                    break;
                case 'A':
                    payload = odroid.getc();
                    odroid.printf("!");
                    liftTo(payload);
                    break;
                case 'H':
                    payload = odroid.getc();
                    odroid.printf("!");
                    wristsTo(payload);
                    break;    
            }

        } 
        else 
        {
            //didn't find start character
            odroid.putc('?');
        }
    }
}
 

  
/*int main() {
    wrists.baud(9600);
    platter.baud(9600);
    lift.baud(9600);
    
    
    
    
    while(1)
    {
        wait(10);
        arms=ARMS_OUT;
       // wrists.putc(WRISTS_90);
       // platter.putc(PLATTER_90);
        wait(4);
        arms=ARMS_IN;
        wait(5);
        //platter.putc(PLATTER_180);
        
        lift.putc(LIFT_120);
        wait(3);
        arms=ARMS_OUT;
        wait(4);
        platter.putc(PLATTER_CR);
        lift.putc(LIFT_0);
        wait(3);
        lift.putc(LIFT_120);
        platter.putc(PLATTER_0);
        wait(3);
        arms=ARMS_IN;
        wait(4);
        lift.putc(LIFT_0);
        wait(4);
        arms=ARMS_OUT;
               
    }
}*/

