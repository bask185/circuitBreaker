#include "src/date.h"
#include "src/version.h"
#include "src/macros.h"

#define DEBUG

const int MAX_FLANK   = 100 ;       // to be Deteremened
const int SHORTCIRCUIT_CURRENT = 300/2 ;
const int SAMPLE_RATE = 1 ;
const int nSamples = 5 ;

uint16_t    prevSample ;
uint8_t     counter ;
uint8_t     sensitivity ;
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

    #ifdef DEBUG
    Serial.begin( 115200 ) ;
    Serial.println( version ) ;
    Serial.println( date ) ;
    #endif

    sample = analogRead( adjust ) ;
    sensitivity = map ( sample, 0, 1023, 200, 50 ) ; // anywhere from 0.5A to 2A
    interval = SAMPLE_RATE ;
    #ifdef DEBUG
    Serial.print("sensitivity: ");Serial.print(10*sensitivity); Serial.println("mA") ;
    #endif
}

void shortcircuit()
{
    #ifndef DEBUG
    CLR( PORTB, 1 ) ;                                               // KILL POWER
    SET( PORTB, 0 ) ;                                               // SET LED

    #else DEBUG
    digitalWrite( LED, HIGH ) ;
    digitalWrite( MOSFET, LOW) ;
    Serial.println("ALARM!!!!!") ;
    #endif
    interval = 1000 ;    
}    

void loop()
{
    REPEAT_MS( interval )
    {
        // IF POWER IS OFF
        #ifndef DEBUG
        if( READ( PORTB, 1 ) == 0 )
        #else
        if( digitalRead( MOSFET ) == 0 ) 
        #endif                                    
        {
            #ifndef DEBUG
            SET( PORTB, 1 ) ;                                                   // RE-ENABLE POWER
            CLR( PORTB, 0 ) ;                                                   // CLEAR LED
            #else
            digitalWrite( LED, LOW ) ;
            digitalWrite( MOSFET, HIGH) ;
            Serial.println("power ON" ) ;
            #endif
            restart = true ;
            interval = SAMPLE_RATE ;

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
            
            if( restart == true )                                               // IF shortcircuit is still present after restarting, immediately kill the power again
            {   restart = false ;
                
                if( current >= sensitivity )  shortcircuit() ;                  
            }
            if( average >= sensitivity )  shortcircuit() ; 
        }

    }   END_REPEAT
}
