#ifndef HARDWARE_H_
#define HARDWARE_H_

#ifdef F_CPU
    #define HARDWARE_FREQ (uint32_t)(F_CPU)
    #define HARDWARE_FREQ_MHZ (uint16_t)(F_CPU/1000000)
#else
    #define HARDWARE_FREQ 0
#endif

#if defined (__AVR_ATtiny45__)
    #define HARDWARE_ID (uint16_t)1 // "AVR_ATtiny45"
#elif defined (__AVR_ATtiny85__)
    #define HARDWARE_ID (uint16_t)2 // "AVR_ATtiny85"
#elif defined (__AVR_ATtiny2313__)
    #define HARDWARE_ID (uint16_t)3 // "AVR_ATtiny2313"
#elif defined (__AVR_ATtiny2313A__)
    #define HARDWARE_ID (uint16_t)4 // "AVR_ATtiny2313A"
#elif defined (__AVR_ATmega48__)
    #define HARDWARE_ID (uint16_t)5 // "AVR_ATmega48"
#elif defined (__AVR_ATmega48A__)
    #define HARDWARE_ID (uint16_t)6 // "AVR_ATmega48A"
#elif defined (__AVR_ATmega48P__)
    #define HARDWARE_ID (uint16_t)7 // "AVR_ATmega48P"
#elif defined (__AVR_ATmega8__)
    #define HARDWARE_ID (uint16_t)8 // "AVR_ATmega8"
#elif defined (__AVR_ATmega8U2__)
    #define HARDWARE_ID (uint16_t)9 // "AVR_ATmega8U2"
#elif defined (__AVR_ATmega88__)
    #define HARDWARE_ID (uint16_t)10 // "AVR_ATmega88"
#elif defined (__AVR_ATmega88A__)
    #define HARDWARE_ID (uint16_t)11 // "AVR_ATmega88A"
#elif defined (__AVR_ATmega88P__)
    #define HARDWARE_ID (uint16_t)12 // "AVR_ATmega88P"
#elif defined (__AVR_ATmega88PA__)
    #define HARDWARE_ID (uint16_t)13 // "AVR_ATmega88PA"
#elif defined (__AVR_ATmega16__)
    #define HARDWARE_ID (uint16_t)14 // "AVR_ATmega16"
#elif defined (__AVR_ATmega168__)
    #define HARDWARE_ID (uint16_t)15 // "AVR_ATmega168"
#elif defined (__AVR_ATmega168A__)
    #define HARDWARE_ID (uint16_t)16 // "AVR_ATmega168A"
#elif defined (__AVR_ATmega168P__)
    #define HARDWARE_ID (uint16_t)17 // "AVR_ATmega168P"
#elif defined (__AVR_ATmega32__)
    #define HARDWARE_ID (uint16_t)18 // "AVR_ATmega32"
#elif defined (__AVR_ATmega328__)
    #define HARDWARE_ID (uint16_t)19 // "AVR_ATmega328"
#elif defined (__AVR_ATmega328P__)
    #define HARDWARE_ID (uint16_t)20 // "AVR_ATmega328P"
#elif defined (__AVR_ATmega32U2__)
    #define HARDWARE_ID (uint16_t)21 // "AVR_ATmega32U2"
#elif defined (__AVR_ATmega32U4__)
    #define HARDWARE_ID (uint16_t)22 // "AVR_ATmega32U4"
#elif defined (__AVR_ATmega32U6__)
    #define HARDWARE_ID (uint16_t)23 // "AVR_ATmega32U6"
#elif defined (__AVR_ATmega128__)
    #define HARDWARE_ID (uint16_t)24 // "AVR_ATmega128"
#elif defined (__AVR_ATmega1280__)
    #define HARDWARE_ID (uint16_t)25 // "AVR_ATmega1280"
#elif defined (__AVR_ATmega2560__)
    #define HARDWARE_ID (uint16_t)26 // "AVR_ATmega2560"
#else
    #define HARDWARE_ID (uint16_t)0 // "Unknown"
#endif

#endif /* HARDWARE_H_ */
