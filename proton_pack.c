/*
 * (C) Copyright 2009, Dave McCaldon <davem1972@gmail.com>
 * All Rights Reserved.
 *
 * proton_pack.c
 */

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/**
 * EEPROM storage for the current track
 */
uint16_t eepromTrack __attribute__((section(".eeprom")));

/**
 * Macro to induce a short delay (2 clock cycles)
 */
#define shortdelay()	\
	__asm__(			\
		"nop\n\t" 		\
		"nop\n\t"  		\
		);
		
/*
	For the 10 blue LEDs on the proton pack power gauge, we have:

	B7		B6		B5		B4		B3		B2		B1		B0		D6		D5
	512		256		128		64		32		16		8		4		2		1
	
	And for the 4 red LEDs on the accelerator:
	
	A1		A0		D1		D0
	2		1		2		1
*/

/**
 * Data to display a sin waveform
 */
uint16_t sin_wave[] PROGMEM = {
	0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff,            // 1
	0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff,                          // 9
	 0xff,  0xff,  0xff,  0xff,                                 // 14
	 0x7f,  0x7f,  0x7f,  0x7f,                                 // 18
	 0x3f,  0x3f,  0x3f,                                        // 22
	 0x1f,  0x1f,  0x1f,                                        // 25
	  0xf,   0xf,   0xf,                                        // 28
	  0x7,   0x7,   0x7,   0x7,                                 // 31
	  0x3,   0x3,   0x3,   0x3,                                 // 35
	  0x1,   0x1,   0x1,   0x1,   0x1,                          // 39
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,     // 44
	  0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,            // 52
	  0x1,   0x1,   0x1,   0x1,   0x1,                          // 59
	  0x3,   0x3,   0x3,   0x3,                                 // 64
	  0x7,   0x7,   0x7,   0x7,                                 // 68
	  0xf,   0xf,   0xf,                                        // 72
	 0x1f,  0x1f,  0x1f,                                        // 75
	 0x3f,  0x3f,  0x3f,                                        // 78
	 0x7f,  0x7f,  0x7f,  0x7f,                                 // 81
	 0xff,  0xff,  0xff,  0xff,                                 // 85
	0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff,                          // 89
	0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff,     // 94
	0xFFFF
};

/**
 * Data to cycle a LED continuously from left to right
 */
uint16_t cycle[] PROGMEM = {
	  0x1,   0x1,   0x1,   0x1,   0x1,
	  0x2,   0x2,   0x2,   0x2,   0x2,
	  0x4,   0x4,   0x4,   0x4,   0x4,
	  0x8,   0x8,   0x8,   0x8,   0x8,
	 0x10,  0x10,  0x10,  0x10,  0x10,
	 0x20,  0x20,  0x20,  0x20,  0x20,
	 0x40,  0x40,  0x40,  0x40,  0x40,
	 0x80,  0x80,  0x80,  0x80,  0x80,
	0x100, 0x100, 0x100, 0x100, 0x100,
	0x200, 0x200, 0x200, 0x200, 0x200,
	0xFFFF	
};

/** 
 * Cylon centurion sweep, left-right-left-right
 */
uint16_t cylon[] PROGMEM = {
	  0x1,   0x1,   0x1,   0x1,   0x1,
	  0x2,   0x2,   0x2,   0x2,   0x2,
	  0x4,   0x4,   0x4,   0x4,   0x4,
	  0x8,   0x8,   0x8,   0x8,   0x8,
	 0x10,  0x10,  0x10,  0x10,  0x10,
	 0x20,  0x20,  0x20,  0x20,  0x20,
	 0x40,  0x40,  0x40,  0x40,  0x40,
	 0x80,  0x80,  0x80,  0x80,  0x80,
	0x100, 0x100, 0x100, 0x100, 0x100,
	0x200, 0x200, 0x200, 0x200, 0x200,
	0x100, 0x100, 0x100, 0x100, 0x100,
	 0x80,  0x80,  0x80,  0x80,  0x80,
	 0x40,  0x40,  0x40,  0x40,  0x40,
	 0x20,  0x20,  0x20,  0x20,  0x20,
	 0x10,  0x10,  0x10,  0x10,  0x10,
	  0x8,   0x8,   0x8,   0x8,   0x8,
	  0x4,   0x4,   0x4,   0x4,   0x4,
	  0x2,   0x2,   0x2,   0x2,   0x2,
	0xFFFF
};

/**
 * Chomp, starts at the top & bottom and meets in the middle
 */
uint16_t chomp[] PROGMEM = {
    0,     0,     0,     0,     0,     
    0,     0,     0,     0,     0,     
	0x201, 0x201, 0x201, 0x201, 0x201, 
	0x303, 0x303, 0x303, 0x303, 0x303, 
	0x387, 0x387, 0x387, 0x387, 0x387, 
	0x3CF, 0x3CF, 0x3CF, 0x3CF, 0x3CF, 
	0x3FF, 0x3FF, 0x3FF, 0x3FF, 0x3FF, 
	0x3CF, 0x3CF, 0x3CF, 0x3CF, 0x3CF, 
	0x387, 0x387, 0x387, 0x387, 0x387, 
	0x303, 0x303, 0x303, 0x303, 0x303, 
	0x201, 0x201, 0x201, 0x201, 0x201, 
	0xFFFF
};

/**
 * Starts in the middle, expands to the top & bottom
 */
uint16_t unchomp[] PROGMEM = {
    	0,     0,     0,     0,     0,
	    0,     0,     0,     0,     0,
	 0x30,  0x30,  0x30,  0x30,  0x30,
	 0x78,  0x78,  0x78,  0x78,  0x78,
	 0xFC,  0xFC,  0xFC,  0xFC,  0xFC,
	0x1FE, 0x1FE, 0x1FE, 0x1FE, 0x1FE,
	0x3FF, 0x3FF, 0x3FF, 0x3FF, 0x3FF,
	0x1FE, 0x1FE, 0x1FE, 0x1FE, 0x1FE,
	 0xFC,  0xFC,  0xFC,  0xFC,  0xFC,
	 0x78,  0x78,  0x78,  0x78,  0x78,
	 0x30,  0x30,  0x30,  0x30,  0x30,
	0xFFFF
};

/**
 * This is a list of the "tracks" that are available in PROGMEM for "playback"
 * via the LEDs.
 */
uint16_t* tracks[] PROGMEM = {
	sin_wave,
	cycle,
	cylon,
	chomp,
	unchomp,
};

/**
 * State needed globally (i.e. in the interrupt handler)
 */
volatile uint16_t track;
volatile uint8_t  restart;
uint16_t max_tracks;

/**
 * Main entry point.  Note that this never returns, when you get to the end
 * it simply loops forever.
 */
int main (void)
{
	uint8_t			PA;
	uint8_t 		PB;
	uint8_t 		PD;
	unsigned int 	j;
	unsigned int	i;
	uint16_t		value;


	//TIMSK   = 0x00;
	MCUSR  &= 0xF7;		// Clear WDRF Flag
	WDTCSR	= 0x18;		// Set bits so we can clear watchdog timer...
	WDTCSR	= 0x00;		// Clear watchdog timer
	
	// Data direction register: DDRD
	// Set all ports to output *EXCEPT* PA2 (reset pin)
	// Only IO lines D6 and B0-B7 will be used for output.
	DDRA = 2+1;			// PA0, PA1
	DDRB = 255;			// PB0-PB7
	DDRD = 64+32+4+2+1;	// PD6,PD5,PD2 (switch input),PD1,PD0

	// Clear all output
	PORTA = 0;	
	PORTB = 0;
	PORTD = 4;
	
	// Set PD2 for input to INT0
	DDRD &= ~4;
	
	// Set up global state
	track      = 0;
	max_tracks = sizeof(tracks)/sizeof(tracks[0]);
	restart    = 0;

	// Grab the current track from EEPROM, if not set, then start at zero
	track = eeprom_read_word(&eepromTrack);
	if ((track == 0xFFFF) || (track >= max_tracks)) {
		track = 0;
		eeprom_write_word(&eepromTrack, track);
	}

	// Initialize the interrupt handling for PD2
	PCMSK |= (1<<PIND2);					// Use PD2 (pin 6) for interrupts
	MCUCR  = (1<<ISC01) | (0<<ISC00);		// Interrupt on falling edge
	GIMSK |= (1<<INT0);						// Select INT0
	sei();									// Enable interrupts

	// Main program loop
	i = 0;
	for (;; i++)
	{
		// Look for a restart flag
		if (restart) {
			i = 0;
			restart = 0;
		}
		
		// Read the next value from the PROGMEM
		uint16_t* ptr = (uint16_t*)pgm_read_word(&tracks[track]);
		value = pgm_read_word(&ptr[i]);
		if (value == 0xFFFF) {
			restart = 1;
			continue;
		}

		// Determine the output bits from the track input
		PA = 0;
		PB = (uint8_t)((value >> 2) & 0xFF);
		PD = (uint8_t)(value & 3) << 5;
		
		// Oversample 'value' into the 4 bits for the accelerator LEDs
		PA |= (value & (512+256) ? 2 : 0) | (value & (128+64) ? 1 : 0);
		PD |= (value & (32+16+8) ? 2 : 0) | (value & (4+2+1) ? 1 : 0);

		// Now, energize the selected LEDs
		j = 0;
		while ((j < 130) && !restart)	// Delay for each step (130 is an arbitrary timing value)
		{
			// We turn on, then off each LED, controls brightness and thus saves power
			PORTA = PA & (1);
			shortdelay();
			PORTA = 0;

			PORTA = PA & (2);
			shortdelay();
			PORTA = 0;

			PORTD |= PD & (1);
			shortdelay();
			PORTD &= ~1;

			PORTD |= PD & (2);
			shortdelay();
			PORTD &= ~2;

			PORTD |= PD & (32);
			shortdelay();
			PORTD &= ~32;

			PORTD |= PD & (64);
			shortdelay();
			PORTD &= ~64;

			PORTB = PB & (1);
			shortdelay();
			PORTB = 0;

			PORTB = PB & (2);
			shortdelay();
			PORTB = 0;

			PORTB = PB & (4);
			shortdelay();
			PORTB = 0;

			PORTB = PB & (8);
			shortdelay();
			PORTB = 0;

			PORTB = PB & (16);
			shortdelay();
			PORTB = 0;

			PORTB = PB & (32);
			shortdelay();
			PORTB = 0;

			PORTB = PB & (64);
			shortdelay();
			PORTB = 0;

			PORTB = PB & (128);
			shortdelay();
			PORTB = 0;
			
			j++;
		}
	}

	return 0;
}

/**
 * This is a signal handler for SIG_INT0, which is triggered when PD2 goes low,
 * (see above).  Note we must do bounce checking here as each time the button
 * is pressed, we'll get a few transitions before it goes steady.
 */
SIGNAL (SIG_INT0)
{
	int i;
	
	
	// Delay for a short while for contact bounce
	for (i = 0; i < 100; i++) {
		shortdelay();
	}

	// Check to see the button is still pressed
	if (0 == (PIND & 4)) {
		// Switch is depressed
		track++;
		if (track >= max_tracks) track = 0;	
	
		// Remember this ...
		eeprom_write_word(&eepromTrack, track);
	
		// Bail out and start over with the new track at the beginning
		restart = 1;
	}
}

/*
 * End-of-file
 *
 */
