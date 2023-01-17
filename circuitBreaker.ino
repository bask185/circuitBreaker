#include "src/date.h"
#include "src/version.h"
#include "src/macros.h"

// #define DEBUG

const int SAMPLE_RATE = 1 ;
const int nSamples = 3 ;
const int RESTART_TIME = 2500 ;

uint16_t    prevSample ;
uint8_t     counter ;
uint16_t    sensitivity ;
uint16_t    sample ;
uint16_t    current ;
uint16_t    samples[ nSamples ] ;
int16_t     dI ;
int16_t     dImax ;
uint16_t    dIprev ;
uint32_t    interval ;
bool        restart ;
bool        overCurrent ;
uint8_t     counter2 ;
uint8_t     index ;

#ifndef DEBUG   // attiny IO definitions

#define currentSens     A1
#define MOSFET          1
#define LED             0
#define adjust          A3

#else            // NANO IO definitions

#define currentSens     A2
#define MOSFET          A5
#define LED             A4
#define adjust          A1

#endif

void setup()
{
    pinMode(currentSens, INPUT);
	pinMode(MOSFET, OUTPUT);
	pinMode(LED, OUTPUT);
	pinMode(adjust, INPUT);

    interval = SAMPLE_RATE;  
    sample = analogRead( adjust ) ;
    sensitivity = map( sample, 0, 1023, 10, 250 ) ; // anywhere from 0.5A to 4A

    /*
        0.5A MIN
        4A MAX
        Rshunt = 0,33R
        Vmin  = 0,5 x 0,33 = 0.1667V
        Vmax  =   4 x 0,33 = 1,3333V
        ADC max = 0.167 / 5 * 1024 = 34
        ADC max = 1,333 / 5 * 1024 = 273
    */
    
    #ifdef DEBUG
    Serial.print("sensitivity: ");Serial.print(10*sensitivity); Serial.println("mA") ;
    #endif
}

void shortcircuit()
{
    CLR( PORTB, 1 ) ;                                               // KILL POWER
    interval = RESTART_TIME ;    
}    

void loop()
{
    REPEAT_MS( interval )
    {
        // IF POWER IS OFF
        if( READ( PORTB, 1 ) == 0 )                                  
        {
            SET( PORTB, 1 ) ;                                                   // RE-ENABLE POWER
            //CLR( PORTB, 0 ) ;                                                   // CLEAR LED

            interval = SAMPLE_RATE ;
            restart = true ;
            for( int i = 0 ; i < nSamples ; i++ )  samples[ i ] = 0 ;           // whipe all samples
        }

        // IF POWER IS ON
        else 
        {
            current = samples[ index ] = analogRead( currentSens ) ;                      // CALCULATE A RUNNING AVERAGE

            if( ++ index == nSamples ) index = 0 ;

            sample = 0 ;
            for( int i = 0 ; i < nSamples ; i++ )
            {
                sample += samples[i] ;
            }
            uint16_t average = sample / nSamples ;
            
            // if( restart == true )                                               // IF shortcircuit is still present after restarting, immediately kill the power again
            // {   restart = false ;
                
            //     if( current >= sensitivity )  shortcircuit() ;                  
            // }
            if( average >= sensitivity )  shortcircuit() ; 
        }

    }   END_REPEAT
}
