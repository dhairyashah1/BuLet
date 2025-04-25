// Toggle P8.11 for 5 seconds. Based on a script by Douglas G. Henke.

.origin 0
.entrypoint START

#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3		// corresponds to PRU_EVTOUT_0

#define DELAY_SECONDS 5		// adjust this to experiment
#define CLOCK 200000000		// PRU is always clocked at 200MHz
#define CLOCKS_PER_LOOP 4	// loop contains two instructions, one clock each
#define DELAYCOUNT DELAY_SECONDS * CLOCK / CLOCKS_PER_LOOP

START:
	MOV	r1, DELAYCOUNT

DELAY:
	CLR	r30, r30, 15  // Clear pin 11 on header P8
	SUB	r1, r1, 1     // decrement loop counter
	SET	r30, r30, 15  // Set pin 11 on header P8
	QBNE	DELAY, r1, 0  // repeat loop unless zero

        // tell host we're done, then halt
	MOV	R31.b0, PRU0_R31_VEC_VALID | SIGNUM
	HALT
