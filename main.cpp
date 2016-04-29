
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

#define C5 (1<<1)
#define C4 (1<<2)
#define C3 (1<<3)
#define C2 (1<<4)
#define C1 (1<<5)
#define B5 (1<<6)
#define B24 (1<<7)
#define B3 (1<<9)
#define B1 (1<<10)
#define A1 (1<<11)
#define A2 (1<<12)
#define A3 (1<<13)
#define A4 (1<<14)
#define A5 (1<<15)

// no M W Z, numbers start with N, XL/R/U/D for arrows
#define CA A2+A3+A4+A5+B1+B3+C2+C3+C4+C5
#define CB A1+A2+A3+A4+A5+B3+B5+C3+C4+C5
#define CC A1+A2+A3+A4+A5+B1+B5+C1+C5
#define CD A3+A4+A5+B3+B5+C1+C2+C3+C4+C5
#define CE A1+A2+A3+A4+A5+B1+B3+B5+C1+C3+C5
#define CF A1+A2+A3+A4+A5+B1+B3+C1
#define CG A1+A2+A3+A5+B1+B3+B5+C1+C2+C3+C4+C5
#define CH A1+A2+A3+A4+A5+B3+C1+C2+C3+C4+C5
#define CI A1+A5+B1+B24+B3+B5+C1+C5
#define CJ A1+A5+B1+B24+B3+B5+C1
#define CK A1+A2+A3+A4+A5+B24+C1+C5
#define CL C5+B5+A1+A2+A3+A4+A5
#define CN A3+A4+A5+B3+C3+C4+C5
#define CO A1+A2+A3+A4+A5+B1+B5+C1+C2+C3+C4+C5
#define CP A1+A2+A3+A4+A5+B1+B3+C1+C2+C3
#define CQ A1+A2+A3+B1+B3+C1+C2+C3
#define CR A3+A4+A5+B3+C3
#define CS C1+C3+C4+C5+B1+B3+B5+A1+A2+A3+A5
#define CT A1+B1+B24+B3+B5+C1
#define CU A1+A2+A3+A4+A5+B5+C1+C2+C3+C4+C5
#define CV A1+A2+A3+A4+B5+C1+C2+C3+C4
#define CX A1+A2+A4+A5+B3+C1+C2+C4+C5
#define CY A1+B24+B3+B5+C1

#define N0 A1+A2+A3+A4+A5+B1+B5+C1+C2+C3+C4+C5
#define N1 A1+A5+B1+B24+B3+B5+C5
#define N2 A1+A4+A5+B1+B3+B5+C1+C2+C5
#define N3 A1+A3+A5+B1+B3+B5+C1+C2+C3+C4+C5
#define N4 A1+A2+A3+B3+C1+C2+C3+C4+C5
#define N5 A1+A2+A3+A5+B1+B3+B5+C1+C3+C4+C5
#define N6 A1+A2+A3+A4+A5+B1+B3+B5+C1+C3+C4+C5
#define N7 A1+B1+C1+C2+C3+C4+C5
#define N8 A1+A2+A3+A4+A5+B1+B3+B5+C1+C2+C3+C4+C5
#define N9 A1+A2+A3+B1+B3+C1+C2+C3+C4+C5

#define XL A1+A5+B24+C3
#define XR A3+B24+C1+C5
#define XU A2+B1+B24+B3+B5+C2
#define XD A4+B1+B24+B3+B5+C4
#define XM A3+B3+C3

#define ATLAS_WORD {spi.write(CA);spi.write(CT);spi.write(CL);spi.write(CA);spi.write(CS);STCP =1;wait(0.001);STCP =0;}
#define STOP_WORD {spi.write(CS);spi.write(CT);spi.write(CO);spi.write(CP);spi.write(0);STCP =1;wait(0.001);STCP =0;}
#define ATLAS_SCROLL {spi.write(CA);update_leds();wait (0.5);spi.write(CT);update_leds();wait (0.5);spi.write(CL);update_leds();wait (0.5);spi.write(CA);update_leds();wait (0.5);spi.write(CS);update_leds();wait (0.5);spi.write(0);spi.write(0);spi.write(0);spi.write(0);spi.write(0);STCP =1;wait(0.001);STCP =0;}
#define LEFT_WORD {spi.write(XL);spi.write(0);spi.write(XL);spi.write(0);spi.write(XL);STCP =1;wait(0.001);STCP =0;}
#define RIGHT_WORD {spi.write(XR);spi.write(0);spi.write(XR);spi.write(0);spi.write(XR);STCP=1;wait(0.001);STCP=0;}

#define WRISTS_90_LEDS {spi.write(CH);spi.write(0);spi.write(0);spi.write(N9);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define WRISTS_n90_LEDS {spi.write(CH);spi.write(0);spi.write(CX);spi.write(N9);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define WRISTS_0_LEDS {spi.write(CH);spi.write(0);spi.write(0);spi.write(0);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define WRISTS_n180_LEDS {spi.write(CH);spi.write(CX);spi.write(N1);spi.write(N8);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define WRISTS_60_LEDS {spi.write(CH);spi.write(0);spi.write(0);spi.write(N6);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define WRISTS_n120_LEDS {spi.write(CH);spi.write(CX);spi.write(N1);spi.write(N2);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define WRISTS_n30_LEDS {spi.write(CH);spi.write(CX);spi.write(0);spi.write(N3);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define LIFT_0_LEDS {spi.write(XU);spi.write(0);spi.write(0);spi.write(0);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define LIFT_42_LEDS {spi.write(XU);spi.write(0);spi.write(0);spi.write(N4);spi.write(N2);STCP =1;wait(0.001);STCP =0;}
#define LIFT_120_LEDS {spi.write(XU);spi.write(0);spi.write(N1);spi.write(N2);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define PLATTER_0_LEDS {spi.write(CP);spi.write(0);spi.write(0);spi.write(0);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define PLATTER_90_LEDS {spi.write(CP);spi.write(0);spi.write(0);spi.write(N9);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define PLATTER_180_LEDS {spi.write(CP);spi.write(0);spi.write(N1);spi.write(N8);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define PLATTER_270_LEDS {spi.write(CP);spi.write(0);spi.write(N2);spi.write(N7);spi.write(N0);STCP =1;wait(0.001);STCP =0;}
#define PLATTER_CR_LEDS {spi.write(CP);spi.write(XR);spi.write(XR);spi.write(XR);spi.write(XR);STCP =1;wait(0.001);STCP =0;}

#define SQUEEZE_IN_LEDS {spi.write(XR);spi.write(XR);spi.write(0);spi.write(XL);spi.write(XL);STCP =1;wait(0.001);STCP =0;}
#define SQUEEZE_OUT_LEDS {spi.write(XL);spi.write(XL);spi.write(0);spi.write(XR);spi.write(XR);STCP =1;wait(0.001);STCP =0;}
#define RESET_LEDS {spi.write(0);spi.write(0);spi.write(0);spi.write(0);spi.write(0);STCP =1;wait(0.001);STCP =0;}

/*
These control the following commands '>'=recieve '<'=send:
>F[2B]<!<d   drive forwards  [distance in    mm]
>B[2B]<!<d   drive backwards [distance in    mm]

>L[2B]<!<d   turn left       [angle in       degrees]
>R[2B]<!<d   turn right      [angle in       degrees]

>A[1B]<!<l   lift            [0: 0, 1: 42, 2: 120]
>H[1B]<!<w   move wrists     [0: 0, 1: -90, 2: 90, 3:60, 4: -120, 5: -30, 6: U1, 7: U2]

>G[1B]<!<s   squeeze            
>g[1B]<!<s   release         

>T<!         turn Platter continuously
>0<!<p       turn Platter to position 0 (000deg)
>1<!<p       turn Platter to position 1 (090deg)
>2<!<p       turn Platter to position 2 (180deg)
>3<!<p       turn Platter to position 3 (270deg)
>P<!         suck
>p<!         stopSucking
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


SPI spi(p5, p6, p7); // mosi, miso, sclk
DigitalOut STCP (p8);

void update_leds()
{
    STCP =1;
    wait(0.001);
    STCP =0;
}

/*
The output pins need to be 0V for low and 5V for high
The MBED can only ouput 0V for low and 3.3V for high
however the pins are pulled to 5V if left floating
so to set the pins high they are set to inputs
and to set the pins low they are set to low outputs
*/
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

/*
Sets the direction of the motors
If the motors are not turning properly change the polarity
of left and right in their if statements to acount for directional differences
*/
void setDirection(bool left, bool right){
    if(!left){
         setPin(leftMotorDirection, 0);
    }
    else{
         setPin(leftMotorDirection, 1); 
    }
    if(right){
         setPin(rightMotorDirection, 0);
    }
    else{
         setPin(rightMotorDirection, 1);  
    }
}

void setMotorEnable(bool isEnabled)
{
    setPin(enable, !isEnabled);
}

/*
Turns to an angle in degrees
If direction is 1 then turns clockwise
If direction is 0 then turns counter clockwise
*/
void turnBy(float angle, bool direction)
{
    if(direction)
    {
        RIGHT_WORD
    }
    else
    {
        LEFT_WORD
    }
    
    setMotorEnable(1);
    int stepsRemaining=(angle*oneDegree);
    setDirection(direction, !direction);
    
    while(stepsRemaining!=0) {
        setPin(motorStepPin, 1);
        wait(stepTime);
        setPin(motorStepPin, 0);
        wait(stepTime);
        stepsRemaining=stepsRemaining-1;
    }
    setMotorEnable(0);
    RESET_LEDS
}

//-------------------------------------------------DRIVE AND ACCELERATION CODE------------------------------------------------------

/*
Function called on the step ticker to step forward 1
Also checks to see if should start deceleration
Detatched itself from ticker when max steps reached
*/
void step()
{
    stepState = !stepState;
    setPin(motorStepPin, stepState);
    stepsRemaining -= 1;
    if(accelerate && stepsRemaining <= STEPS_TO_DECCELERATE)
    {
        accelerationTimer.attach(decFunction, STEPS_TO_DECCELERATE);    
    }
    if (stepsRemaining <=0)
    {
        stepTimer.detach();
        odroid.putc('d');
        setMotorEnable(0);
    }
}

/*
Increases the step rate when called on acceleration ticker
detatches itself once max speed reached
*/
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

/*
Decreases the step rate on acceleration ticker
detatches itself when end speed reached
*/
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

/*
Sets up drive tickers and acceleration tickers if necessary
Direction is true for forwards and false for backwards
*/
void driveBy(float distance, bool direction)
{
    setMotorEnable(1);
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

/*
Tells pick to move the platter to a position (or contiuously)
*/
void movePlatter(int mode)
{
    platter.putc(mode);
    switch(mode){
        case PLATTER_0:
            PLATTER_0_LEDS
            break;
        case PLATTER_90:
            PLATTER_90_LEDS
            break;
        case PLATTER_180:
            PLATTER_180_LEDS
            break;
        case PLATTER_270:
            PLATTER_270_LEDS
            break;     
    }
}

/*
Called when platter returns
Tells ODROID platter is done
*/
void returnPlatter()
{
    odroid.printf ("p");
    RESET_LEDS
}

/*
Tells pic to bring the arms in or out (squeeze mechanism)
mode
    true: squeeze
    false: release
*/
void setSqueeze(bool mode)
{
    if(mode){
        squeeze = SQUEEZE_IN;
        SQUEEZE_IN_LEDS
    }
    else{
        squeeze = SQUEEZE_OUT;  
        SQUEEZE_OUT_LEDS  
    }  
}

void returnSqueeze()
{
    odroid.printf ("s");
    RESET_LEDS
}

void liftTo(int mode)
{
    lift.putc(mode);  
    switch(mode)
    {
        case LIFT_0:
            LIFT_0_LEDS
            break;
        case LIFT_42:
            LIFT_42_LEDS
            break;
        case LIFT_120:
            LIFT_120_LEDS
            break;   
    }  
}

void returnLift()
{
    odroid.printf ("l"); 
    RESET_LEDS   
}

void wristsTo(int mode)
{
    wrists.putc(mode);  
    switch(mode)
    {
        case WRISTS_0:
            WRISTS_0_LEDS
            break;
        case WRISTS_60:
            WRISTS_60_LEDS
            break;
        case WRISTS_90:
            WRISTS_90_LEDS
            break;
        case WRISTS_n30:
            WRISTS_n30_LEDS
            break;
        case WRISTS_n90:
            WRISTS_n90_LEDS
            break;
        case WRISTS_n120:
            WRISTS_n120_LEDS
            break;
    }        
}

void returnWrists()
{
    odroid.printf ("w");
    RESET_LEDS
}

void setSuck(bool enable)
{
    setPin(pump, !enable);
}

int main() {
    setMotorEnable(0);
    setSuck(0);
    wrists.baud(9600);
    platter.baud(9600);
    lift.baud(9600);
    
    wristsReturn.rise(&returnWrists);
    liftReturn.rise(&returnLift);
    squeezeReturn.rise(&returnSqueeze);
    platterReturn.rise(&returnPlatter);
    
    ATLAS_SCROLL
    
    
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

