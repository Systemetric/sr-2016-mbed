
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

#define LIFT_0              0
#define LIFT_42             1   
#define LIFT_120            2

#define SQUEEZE_IN          0
#define SQUEEZE_OUT         1

#define oneDegree (2500.0/90.0)//was 2650.0-->2026
#define oneMM (2650.0/325.0)
#define stepTime 0.0003
//2650 steps gives 180 degrees and 650mm straight line
//2650 steps now gives 90 degrees and 325mm straight line

#define START_STEPS_PER_SECOND 1000
#define MAX_STEPS_PER_SECOND 9000
#define FINISH_STEPS_PER_SECOND 1000

#define STEPS_PER_SECOND_PER_SECOND 500
#define STEPS_TO_INCREMENT 2

#define STEPS_TO_ACCELERATE (MAX_STEPS_PER_SECOND * MAX_STEPS_PER_SECOND - START_STEPS_PER_SECOND * START_STEPS_PER_SECOND) / (2 * STEPS_TO_INCREMENT * STEPS_PER_SECOND_PER_SECOND)
#define STEPS_TO_DECCELERATE (MAX_STEPS_PER_SECOND * MAX_STEPS_PER_SECOND - FINISH_STEPS_PER_SECOND * FINISH_STEPS_PER_SECOND) / (2 * STEPS_TO_INCREMENT * STEPS_PER_SECOND_PER_SECOND)

#define TIME_BETWEEN_INCREMENTS 1000000/STEPS_PER_SECOND_PER_SECOND

/*
These control the following commands '>'=recieve '<'=send:
>F[2B]<!<d   drive forwards  [distance in    mm]
>B[2B]<!<d   drive backwards [distance in    mm]

>L[2B]<!<d   turn left       [angle in       degrees]
>R[2B]<!<d   turn right      [angle in       degrees]

>A[1B]<!<d   move squeeze       [0: 0, 1: 42, 2: 120]
>H[1B]<!<d   move hands      [0: 0, 1: -90, 2: 90, 3:60, 4: -120, 5: -30, 6: U1, 7: U2]

>G[1B]<!     squeeze            
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
DigitalInOut leftMotorDirection(p21);//These have a different header instead of power-pulse-direction it is power-direction-pulse (does not affect code)
DigitalInOut enable(p23); //active low
DigitalInOut rightMotorDirection(p24);//These have a different header instead of power-pulse-direction it is power-direction-pulse (does not affect code)
Serial odroid(USBTX, USBRX);
//DigitalOut PlatterCR(p20);
//DigitalOut Platter0(p19);
//DigitalOut Platter1(p18);
//DigitalOut Platter2(p17);
//DigitalOut Platter3(p16);
Serial wrists(p28, p27);  // tx, rx
InterruptIn wristsReturn(p29);//guessed at value
Serial platter(p9, p10);  // tx, rx
InterruptIn platterReturn(p11);//guessed at value
Serial lift  (p13, p14);
InterruptIn liftReturn(p15);//guessed at value
DigitalOut squeeze (p12);
InterruptIn squeezeReturn(p16);//guessed at value
DigitalInOut pump(p25);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);



Ticker stepTimer;
Ticker accelerationTimer;
bool stepState;
bool accelerate;
int driveStepsRemaining;
int stepsPerSecond;
int stepsRemaining;
int stepsRequired;
void(*decFunction)(void);

int currentPlatter = 0;

void setPin(DigitalInOut pin, bool state)
{
    if (state)
    {
        pin.input();
    }
    else
    {
        pin.output();
        pin = 0;
    }    
}

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

void step()
{
    stepState = !stepState;
    motorStep(stepState);
    stepsRemaining -= 1;
    if(accelerate && stepsRemaining <= STEPS_TO_DECCELERATE)
    {
        accelerationTimer.attach(decFunction, STEPS_TO_DECCELERATE);    
    }
    if (stepsRemaining <=0)
    {
        stepTimer.detach();
        odroid.putc('d');
    }
}

void incrementStepsPerSecond()
{
    if (stepsRemaining>stepsRequired - STEPS_TO_ACCELERATE)
    {
        stepsPerSecond += STEPS_TO_INCREMENT;
        float timeBetweenSteps = 1000000 / stepsPerSecond;
        
        stepTimer.detach ();
        stepTimer.attach_us(&step, timeBetweenSteps);
    }
    else{
        accelerationTimer.detach();
    }
}

void decrementStepsPerSecond()
{
    if (stepsRemaining>0)
    {
        stepsPerSecond -= STEPS_TO_INCREMENT;
        float timeBetweenSteps = 1000000 / stepsPerSecond;
        
        stepTimer.detach();
        stepTimer.attach_us(&step, timeBetweenSteps);
    }
    else{
        accelerationTimer.detach();
    }
}

void driveBy(float distance, bool direction)
{
    stepsRequired = distance*oneMM;
    stepsRemaining = stepsRequired;
    stepState = 0;
    decFunction = &decrementStepsPerSecond;
    setDirection(direction, direction);
    
    stepsPerSecond = START_STEPS_PER_SECOND;
    float timeBetweenSteps = 1000000/stepsPerSecond;
    
    stepTimer.attach_us(&step, timeBetweenSteps);
    accelerate = stepsRequired>STEPS_TO_ACCELERATE+STEPS_TO_DECCELERATE;
    if (accelerate)
    {
        accelerationTimer.attach_us(&incrementStepsPerSecond, TIME_BETWEEN_INCREMENTS);
    }
}

void movePlatter(int mode)
{
    platter.putc(mode);
}

void returnPlatter()
{
    odroid.printf ("p");
}

//true: squeeze
//false: release
void setSqueeze(bool mode)
{
    if(mode){
        squeeze = SQUEEZE_IN;
    }
    else{
        squeeze = SQUEEZE_OUT;    
    }  
}

void returnSqueeze()
{
    odroid.printf ("s");
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

void returnLift()
{
    odroid.printf ("l");    
}

void wristsTo(int mode)
{
    wrists.putc(mode);    
}

void returnWrists()
{
    odroid.printf ("w");
}

void setSuck(bool enable)
{
    setPin(pump, !enable);
}

int main() {
    setSuck(0);
    wrists.baud(9600);
    platter.baud(9600);
    lift.baud(9600);
    
    wristsReturn.rise(&returnWrists);
    liftReturn.rise(&returnLift);
    squeezeReturn.rise(&returnSqueeze);
    platterReturn.rise(&returnPlatter);
    
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
                    movePlatter(currentPlatter);
                    break;
                case '0':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_0;
                    movePlatter(currentPlatter);
                    break;
                case '1':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_90;
                    movePlatter(currentPlatter);
                    break;
                case '2':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_180;
                    movePlatter(currentPlatter);
                    break;
                case '3':
                    odroid.printf ("!");
                    currentPlatter = PLATTER_270;
                    movePlatter(currentPlatter);
                    break;
                case 'P':
                    odroid.printf ("!");
                    setSuck(1);
                    break;
                case 'p':
                    odroid.printf ("!");
                    setSuck(0);
                    break;
                case 'G':
                    odroid.printf("'");
                    setSqueeze(true);
                    break;
                case 'g':
                    odroid.printf("!");
                    setSqueeze(false);
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

