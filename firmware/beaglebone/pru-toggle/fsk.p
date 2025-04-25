.origin 0 
.entrypoint START

#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0
#define     MAIN_INSTRUCTIONS 4

#define     ntoggle r1
#define     delay   r2
#define     counter r3
#define     offset  r4


/////////////////////////////////////////////////////////////////////////////
// Initialize all registers to 0, then jump to the main loop.
START:
    MOV     ntoggle, 0
    MOV     delay, 0
    MOV     counter, 0
    MOV     offset, 0
    CLR     r30, r30, 15            // Clear the pin
    JMP     MAIN
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// This is the toggle loop, which flips the GPIO in each iteration.
// The loop executes `ntoggle` flips in total.
// Each iteration, except for the last, will take
//          6+2*delay instructions
// The last iteration will take
//          6+2*(delay-MAIN_INSTRUCTIONS) instructions

TOGGLE:
    XOR     r30.b1, r30.b1, (1 << 7)                // |--- 1 instruction

SETUP_DELAY:
    MOV     counter, delay                          // |--- 2 instructions
    SUB     ntoggle, ntoggle, 1                     // |---
    
    QBEQ    LAST_ITERATION, ntoggle, 0              // |---
    JMP     DELAY                                   // | 2 instructions
LAST_ITERATION:                                     // |
    SUB     counter, counter, MAIN_INSTRUCTIONS     // |---

DELAY:                                              // |---
    SUB     counter, counter, 1                     // | 2*initial counter value instructions
    QBNE    DELAY, counter, 0                       // |---

    QBNE    TOGGLE, ntoggle, 0                      // |--- 1 instruction
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// This is the main loop
//
// It reads tuples of (ntoggle, delay) from the PRU RAM.
// If ntoggle is not 0, it calls the toggle loop to toggle the GPIO
// `ntoggle` times with a delay of `(6+delay)*5` ns between each
// toggle even.
// If ntoggle is 0, it terminates.
MAIN:
    LBBO    ntoggle, offset, #0x00, 1
    LBBO    delay, offset, #0x01, 1
    ADD     offset, offset, 2
    QBNE    TOGGLE, ntoggle, 0
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Clean up. Reached by fall-through.
END:
    CLR    r30, r30, 15            // Clear the pin
    MOV    R31.b0, PRU0_R31_VEC_VALID | SIGNUM
    HALT
/////////////////////////////////////////////////////////////////////////////
