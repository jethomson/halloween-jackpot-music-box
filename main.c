/*----------------------------------------------------------------------------/
/  20-pin SD Sound Generator R0.01                                            /
/-----------------------------------------------------------------------------/
/ This project, program codes and circuit diagrams, is opened under license
/ policy of following trems.
/
/  Copyright (C) 2010, ChaN, all right reserved.
/  All modifications to ChaN's work are Copyright 2016, Jonathan Thomson.
/
/ * This project is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/----------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "pff.h"

#ifndef MODE
#error Wrong make file.
#endif

#define FCC(c1,c2,c3,c4)	(((DWORD)c4<<24)+((DWORD)c3<<16)+((WORD)c2<<8)+(BYTE)c1)	/* FourCC */

#define STATUSLED_ON()	PORTB |= _BV(2)
#define STATUSLED_OFF()	PORTB &= ~_BV(2)

#define GREEN_ON()	PORTA |= _BV(3)
#define GREEN_OFF()	PORTA &= ~_BV(3)
#define GREEN_TOGGLE()	PORTA ^= _BV(3)

#define RED_ON()	PORTA |= _BV(4)
#define RED_OFF()	PORTA &= ~_BV(4)
#define RED_TOGGLE()	PORTA ^= _BV(4)

#define ORANGE_ON()	PORTA |= _BV(7)
#define ORANGE_OFF()	PORTA &= ~_BV(7)
#define ORANGE_TOGGLE()	PORTA ^= _BV(7)

void delay_ms (WORD);	/* Defined in asmfunc.S */
void delay_us (WORD);	/* Defined in asmfunc.S */

//EMPTY_INTERRUPT(PCINT_vect);


/*---------------------------------------------------------*/
/* Work Area                                               */
/*---------------------------------------------------------*/

volatile BYTE FifoRi, FifoWi, FifoCt;	/* FIFO controls */
BYTE Buff[256];		/* Audio output FIFO */

BYTE InMode, Cmd;	/* Input mode and received command value */

FATFS Fs;			/* File system object */
DIR Dir;			/* Directory object */
FILINFO Fno;		/* File information */

volatile unsigned char led_effects_enabled = 1;
volatile unsigned char skip_to_next = 0;

WORD rb;			/* Return value. Put this here to avoid avr-gcc's bug */


/*---------------------------------------------------------*/
/* Sub-routines                                            */
/*---------------------------------------------------------*/


static
void led_sign (
	BYTE ct		/* Number of flashes */
)
{
	do {
		delay_ms(200);
		STATUSLED_ON();
		delay_ms(100);
		STATUSLED_OFF();
	} while (--ct);
	delay_ms(1000);
}



static void led_effect0 (void)
{
	unsigned char i = 0;
	while (led_effects_enabled) {
		GREEN_ON();
		for (i = 0; i < 6 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;
		GREEN_OFF();

		RED_ON();
		for (i = 0; i < 6 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;
		RED_OFF();

		ORANGE_ON();
		for (i = 0; i < 6 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;
		ORANGE_OFF();

		RED_ON();
		for (i = 0; i < 6 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;
		RED_OFF();
	}
}


static void led_effect1 (void)
{
	unsigned char i = 0;
	while (led_effects_enabled) {
		GREEN_ON();
		RED_ON();
		ORANGE_ON();
		for (i = 0; i < 6 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;

		GREEN_OFF();
		RED_OFF();
		ORANGE_OFF();
		for (i = 0; i < 3 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;

		GREEN_ON();
		RED_ON();
		ORANGE_ON();
		for (i = 0; i < 6 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;

		GREEN_OFF();
		RED_OFF();
		ORANGE_OFF();
		for (i = 0; i < 3 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;

		GREEN_ON();
		RED_ON();
		ORANGE_ON();
		for (i = 0; i < 6 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;

		GREEN_OFF();
		RED_OFF();
		ORANGE_OFF();
		for (i = 0; i < 12 && led_effects_enabled; i++) {
			delay_ms(50);
		}
		if (!led_effects_enabled) break;
	}
}

static void led_effect2 (void)
{
	unsigned int i = 0;
	unsigned int j = 0;
	while (led_effects_enabled) {
		for (i = 1; led_effects_enabled; i++) {
			if (i > 750) i+=2;
			if (i > 1250) i+=4;
			if (i >= 2000) break;

			GREEN_ON();
			RED_ON();
			ORANGE_ON();
			j = i;
			while (j > 0 && led_effects_enabled) {
				delay_us(1);
				j--;
			}
			if (!led_effects_enabled) break;

			GREEN_OFF();
			RED_OFF();
			ORANGE_OFF();
			j = 2000-i;
			while (j > 0 && led_effects_enabled) {
				delay_us(1);
				j--;
			}
		}
		GREEN_ON();
		RED_ON();
		ORANGE_ON();
		if (!led_effects_enabled) break;
		for (i = 1999; led_effects_enabled; i--) {
			if (i > 750) i-=2;
			if (i > 1250) i-=4;
			if (i <= 0) break;

			GREEN_ON();
			RED_ON();
			ORANGE_ON();
			j = i;
			while (j > 0 && led_effects_enabled) {
				delay_us(1);
				j--;
			}
			if (!led_effects_enabled) break;

			GREEN_OFF();
			RED_OFF();
			ORANGE_OFF();
			j = 2000-i;
			while (j > 0 && led_effects_enabled) {
				delay_us(1);
				j--;
			}
		}
		GREEN_OFF();
		RED_OFF();
		ORANGE_OFF();
		if (!led_effects_enabled) break;
		delay_ms(100);
	}
}

static void led_effect3 (void)
{
	unsigned char j = 0;
	unsigned char rand = (42*109+89)%251; // seed with 42
	while (led_effects_enabled) {
		GREEN_ON();
		RED_ON();
		ORANGE_ON();
		j = rand;
		rand = (rand*109+89)%251;
		while (j > 0 && led_effects_enabled) {
			delay_ms(1);
			j--;
		}
		if (!led_effects_enabled) break;

		GREEN_OFF();
		RED_OFF();
		ORANGE_OFF();
		j = rand;
		rand = (rand*109+89)%251;
		while (j > 0 && led_effects_enabled) {
			delay_ms(1);
			j--;
		}
	}
}

static void led_effects_handler (void)
{
	static unsigned char effect_num = 3;
	switch (effect_num) {
	case 0:
		effect_num = 1;
		led_effect0();
		break;
	case 1:
		effect_num = 2;
		led_effect1();
		break;
	case 2:
		effect_num = 3;
		led_effect2();
		break;
	case 3:
		effect_num = 0;
		led_effect3();
		break;
	default:
		effect_num = 0;
		led_effect0();
	}
	GREEN_OFF();
	RED_OFF();
	ORANGE_OFF();
}

static
BYTE chk_input (void)	/* 0:Not changed, 1:Changed */
{
	BYTE k, n;
	static BYTE pk, nk;

	k = ~(PINB >> 4) & 0b00000111;
	n = nk; nk = k;
	if (n != k || pk == k) return 0;

	pk = k; Cmd = k;

	return 1;
}



static
void ramp (		/* Ramp-up/down audio output (anti-pop feature) */
	int dir		/* 0:Ramp-down, 1:Ramp-up */
)
{
#if MODE != 0	/* This function is enebled on non-OCL output cfg. */
	BYTE v, d, n;


	if (dir) {
		v = 0; d = 1;
	} else {
		v = 128; d = 0xFF;
	}

	n = 128;
	do {
		v += d;
		OCR1A = v; OCR1B = v;
		delay_us(100);
	} while (--n);
#endif
}



static
void audio_on (void)	/* Enable audio output functions */
{
	if (!TCCR0B) {
		FifoCt = 0; FifoRi = 0; FifoWi = 0;		/* Reset audio FIFO */
		PLLCSR = 0b00000110;	/* Select PLL clock for TC1.ck */
		TCCR1A = 0b10100011;	/* Start TC1 with OC1A/OC1B PWM enabled */
		TCCR1B = 0b00000001;
		ramp(1);				/* Ramp-up to center level */
		TCCR0A = 0b00000001;	/* Enable TC0.ck = 2MHz as interval timer */
		TCCR0B = 0b00000010;
		TIMSK = _BV(OCIE0A);
	}
}



static
void audio_off (void)	/* Disable audio output functions */
{
	if (TCCR0B) {
		TCCR0B = 0;				/* Stop audio timer */
		ramp(0);				/* Ramp-down to GND level */
		TCCR1A = 0;	TCCR1B = 0;	/* Stop PWM */
	}
}



static
void wait_status (void)	/* Wait for a code change */
{
	BYTE n;


	if (Cmd) return;

	audio_off();	/* Disable audio output */

	for (;;) {
		n = 10;				/* Wait for a code change at active mode (100ms max) */
		do {
			delay_ms(10);
			chk_input();
		} while (--n && !Cmd);
		if (Cmd) break;		/* Return if any code change is detected within 100ms */

		led_effects_enabled = 1;
		sei();
		led_effects_handler();
	}
}



static
DWORD load_header (void)	/* 2:I/O error, 4:Invalid file, >=1024:Ok(number of samples) */
{
	DWORD sz, f;
	BYTE b, al = 0;


	/* Check RIFF-WAVE file header */
	if (pf_read(Buff, 12, &rb)) return 2;
	if (rb != 12 || LD_DWORD(Buff+8) != FCC('W','A','V','E')) return 4;

	for (;;) {
		if (pf_read(Buff, 8, &rb)) return 2;		/* Get Chunk ID and size */
		if (rb != 8) return 4;
		sz = LD_DWORD(&Buff[4]);		/* Chunk size */

		switch (LD_DWORD(&Buff[0])) {	/* Switch by chunk type */
		case FCC('f','m','t',' ') :		/* 'fmt ' chunk */
			if (sz & 1) sz++;
			if (sz > 128 || sz < 16) return 4;		/* Check chunk size */
			if (pf_read(Buff, sz, &rb)) return 2;	/* Get the chunk content */
			if (rb != sz) return 4;
			if (Buff[0] != 1) return 4;				/* Check coding type (1: LPCM) */
			b = Buff[2];
			if (b < 1 && b > 2) return 4; 			/* Check channels (1/2: Mono/Stereo) */
			GPIOR0 = al = b;						/* Save channel flag */
			b = Buff[14];
			if (b != 8 && b != 16) return 4;		/* Check resolution (8/16 bit) */
			GPIOR0 |= b;							/* Save resolution flag */
			if (b & 16) al <<= 1;
			f = LD_DWORD(&Buff[4]);					/* Check sampling freqency (8k-48k) */
			if (f < 8000 || f > 48000) return 4;
			OCR0A = (BYTE)(16000000UL/8/f) - 1;		/* Set interval timer (sampling period) */
			break;

		case FCC('d','a','t','a') :		/* 'data' chunk (start to play) */
			if (!al) return 4;							/* Check if format valid */
			if (sz < 1024 || (sz & (al - 1))) return 4;	/* Check size */
			if (Fs.fptr & (al - 1)) return 4;			/* Check offset */
			return sz;

		case FCC('D','I','S','P') :		/* 'DISP' chunk (skip) */
		case FCC('f','a','c','t') :		/* 'fact' chunk (skip) */
		case FCC('L','I','S','T') :		/* 'LIST' chunk (skip) */
			if (sz & 1) sz++;
			if (pf_lseek(Fs.fptr + sz)) return 2;
			break;

		default :						/* Unknown chunk */
			return 4;
		}
	}
}



static
BYTE play (		/* 0:Normal end, 1:Continue to play, 2:Disk error, 3:No file, 4:Invalid file */
	BYTE dn		/* File number (1..255) */
)
{
	DWORD sz, spa, sza;
	FRESULT res;
	char *dir = "";
	WORD btr;
	BYTE rc;
	static unsigned char win_filenum = 0;
	static unsigned char fail_filenum = 0;
	static unsigned char scary_filenum = 0;
	unsigned char *filenum;
	unsigned char filenum_cnt = 0;

	skip_to_next = 0;

	/*
	 * pressed SW1 --> PB4 LOW --> dn = 1
	 * pressed SW2 --> PB5 LOW --> dn = 2
	 * pressed SW3 --> PB6 LOW --> dn = 4
	 */
	if (dn == 1) { //001
		GREEN_ON();
		dir = "win";
		filenum = &win_filenum;
	}
	else if (dn == 2) { //010
		RED_ON();
		dir = "fail";
		filenum = &fail_filenum;
	}
	else if (dn == 4) { //100
		ORANGE_ON();
		dir = "scary";
		filenum = &scary_filenum;
	}
	else {
		return 2; // invalid input
	}

	res = pf_opendir(&Dir, dir);	/* Open sound file directory */
	if (res == FR_NO_PATH) return 3;
	if (res != FR_OK) return 2;

	if (res == FR_OK) {
		for (filenum_cnt = 0; filenum_cnt <= *filenum; filenum_cnt++) {
			res = pf_readdir(&Dir, &Fno);		/* Get a dir entry */
			if (res || Fno.fname[0] == 0) return 2;	/* Return on error or end of dir */
		}

		if (!(Fno.fattrib & (AM_DIR|AM_HID)) && strstr(Fno.fname, ".WAV")) {
			strcpy((char*)Buff, dir);
			strcat((char*)Buff, "/");
			strcat((char*)Buff, Fno.fname);
		}
	}

	res = pf_open((char*)Buff);		/* Open sound file */
	if (res == FR_NO_FILE) return 3;
	if (res != FR_OK) return 2;

	/* Get file parameters */
	sz = load_header();
	if (sz <= 4) return (BYTE)sz;	/* Invalid format */
	spa = Fs.fptr; sza = sz;		/* Save offset and size of audio data */

	//STATUSLED_ON();
	audio_on();		/* Enable audio output */

	*filenum = *filenum+1;
	/* If at end of directory, reset filenum to zero */
	res = pf_readdir(&Dir, &Fno);		/* Get a dir entry */
	if (Fno.fname[0] == 0) {	/* End of dir */
		*filenum = 0;
	}

	for (;;) {
		if (pf_read(0, 512 - (Fs.fptr % 512), &rb) != FR_OK) {		/* Snip sector unaligned part */
			rc = 2; break;
		}
		sz -= rb;
		do {
			/* Forward a bunch of the audio data to the FIFO */
			btr = (sz > 1024) ? 1024 : (WORD)sz;
			pf_read(0, btr, &rb);
			if (btr != rb) {
				rc = 2; break;
			}
			sz -= rb;

			/* Check input code change */
			rc = 0;
			if (chk_input()) {
				switch (InMode) {
				case 3: 	/* Mode 3: Edge triggered (retriggerable) */
					if (Cmd) rc = 1;	/* Restart by a code change but zero */
					break;
				case 2:		/* Mode 2: Edge triggered */
					Cmd = 0;			/* Ignore code changes while playing */
					break;
				case 1:		/* Mode 1: Level triggered (sustained to end of the file) */
					if (Cmd && Cmd != dn) rc = 1;	/* Restart by a code change but zero */
					break;
				default:	/* Mode 0: Level triggered */
					if (Cmd != dn) rc = 1;	/* Restart by a code change */
				}
			}
		} while (!rc && rb == 1024);	/* Repeat until all data read or code change */

		if (rc || !Cmd || InMode >= 2) break;
		if (pf_lseek(spa) != FR_OK) {	/* Return top of audio data */
			rc = 3; break;
		}
		sz = sza;
	}

	while (FifoCt) ;			/* Wait for audio FIFO empty */
	OCR1A = 0x80; OCR1B = 0x80;	/* Return DAC out to center */

	//STATUSLED_OFF();
	GREEN_OFF();
	RED_OFF();
	ORANGE_OFF();
	cli(); // mode 3 doesn't work correctly when re-triggering if this isn't here

	return rc;
}



/*-----------------------------------------------------------------------*/
/* Main                                                                  */

int main (void)
{
	BYTE rc;


	MCUSR = 0;								/* Clear reset status */
	PCMSK0 = 0b00000000;					/* Select pin change interrupt pins PB4, PB5, PB6 (SW1..SW3) */
	PCMSK1 = 0b01110000;


	/* Initialize ports */ // LEDs on PB3, PB4, and PB7
	PORTA = 0b01100011;		/* PORTA [LppLLLHp]*/
	DDRA  = 0b10011110;
	PORTB = 0b01110001;		/* PORTB [-pppLLLH] */
	DDRB  = 0b00001111;

	GIMSK = _BV(PCIE1);
	sei();

	for (;;) {

		if (pf_mount(&Fs) == FR_OK) {	/* Initialize FS */

			/* Load command input mode (if not exist, use mode 0 as default) */
			strcpy_P((char*)Buff, PSTR("000.TXT"));
			if (pf_open((char*)Buff) == FR_OK) {
				pf_read(&InMode, 1, &rb);
				InMode -= '0';
			}

			/* Main loop */
			do {
				wait_status();				/* Wait for any valid code */
				rc = play(Cmd);				/* Play corresponding audio file */
				if (rc >= 2) led_sign(rc);	/* Display if any error occured */
				if (rc != 1) Cmd = 0;		/* Clear code when normal end or error */
			} while (rc != 2);				/* Continue while no disk error */

			audio_off();	/* Disable audio output */
		}
		led_sign(2);	/* Disk error or Media mount failed */
	}
}

ISR (PCINT_vect)
{
	//STATUSLED_ON();
	BYTE k;
	k = ~(PINB >> 4) & 0b00000111;

	if (k) {
		led_effects_enabled = 0;
	}
}
