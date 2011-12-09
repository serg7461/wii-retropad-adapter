#include <WProgram.h>
#include "digitalWriteFast.h"
#include "GCPad.h"

// DO NOT CHANGE PIN DEFINITION BELOW!!!
// GCPad_recv doesn't use digitalReadFast(), it's hardcoded there!
// To be fixed someday...
#define JOY_DATA_PIN 2

byte raw_joy_data[64];
byte gc_joy_data[8];
byte n64_joy_data[4];

/* DO NOT CHANGE ANYTHING IN THE FUNCTION BELOW!!!
 *
 * It was specially crafted to work with an 8Mhz Atmega328/168 (int. resonator).
 *
 * */
void GCPad_send(byte *cmd, byte length) {
	byte bit = 128;
	byte high;

	pinModeFast(JOY_DATA_PIN, OUTPUT);

	loop:

	high = *cmd & bit;

	digitalWriteFast(JOY_DATA_PIN, LOW);

	if(high) {
		digitalWriteFast(JOY_DATA_PIN, HIGH);

		bit >>= 1;

		if(bit < 1) {
			bit = 128;
			cmd++;
			length--;
		} else {
			asm volatile ("nop\nnop\nnop\nnop\nnop\n");
		}

		asm volatile ("nop\nnop\nnop\nnop\n");
	} else {
		asm volatile ("nop\nnop\nnop\n");

		bit >>= 1;

		if(bit < 1) {
			bit = 128;
			cmd++;
			length--;
		} else {
			asm volatile ("nop\nnop\nnop\nnop\nnop\n");
		}

		digitalWriteFast(JOY_DATA_PIN, HIGH);

		asm volatile ("nop\nnop\n");
	}

	if(length > 0) goto loop;

	asm volatile ("nop\n");

	// Final stop bit (1)

	digitalWriteFast(JOY_DATA_PIN, LOW);
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\n");
	digitalWriteFast(JOY_DATA_PIN, HIGH);
}

/* DO NOT CHANGE ANYTHING IN THE FUNCTION BELOW!!!
 *
 * It was specially crafted to work with an 8Mhz Atmega328/168 (int. resonator).
 *
 * */
void GCPad_recv(byte *buffer, byte bits) {

	pinModeFast(JOY_DATA_PIN, INPUT);
	digitalWriteFast(JOY_DATA_PIN, HIGH);

	loop:

	while(PIND & 0x04);


	asm volatile (
			"nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
			"nop\nnop\nnop\nnop\nnop\n"
	);

	*buffer = PIND & 0x04; //*buffer = digitalReadFast(JOY_DATA_PIN);

	buffer++;
	bits--;

	if(bits == 0)
		return;

	while(!(PIND & 0x04));

    goto loop;
}

void GCPad_init() {
	byte init = 0x00;

	for(int x = 0; x < 64; x++) {
		raw_joy_data[x] = 0x00;
	}

	for(int x = 0; x < 8; x++) {
		gc_joy_data[x] = 0x00;
	}

	for(int x = 0; x < 4; x++) {
		n64_joy_data[x] = 0x00;
	}

	noInterrupts();
	GCPad_send(&init, 1);
	interrupts();

	pinModeFast(JOY_DATA_PIN, INPUT);

	for (int x=0; x<64; x++) {
		if (!(PIND & 0x04))
			x = 0;
	}
}

byte *GCPad_read() {
	int bit;

	byte cmd[3] = {0x40, 0x03, 0x00};

	noInterrupts();

	GCPad_send(cmd, 3);
	GCPad_recv(raw_joy_data, 64);

	interrupts();

	bit = 7;

	for(int i = 0; i < 8; i++) {
		for(int j = 0; j < 8; j++) {

			if(raw_joy_data[8 * i + j] != 0 ) {
				gc_joy_data[i] |= (1 << bit);
			} else {
				gc_joy_data[i] &= ~(1 << bit);
			}

			bit--;

			if(bit < 0)
				bit = 7;
		}
	}

	return gc_joy_data;
}

byte *N64Pad_read() {
	int bit;

	byte cmd[1] = {0x01};

	noInterrupts();

	GCPad_send(cmd, 1);
	GCPad_recv(raw_joy_data, 32);

	interrupts();

	bit = 7;

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 8; j++) {

			if(raw_joy_data[8 * i + j] != 0 ) {
				n64_joy_data[i] |= (1 << bit);
			} else {
				n64_joy_data[i] &= ~(1 << bit);
			}

			bit--;

			if(bit < 0)
				bit = 7;
		}
	}

	return n64_joy_data;
}
