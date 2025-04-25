.origin 0 
.entrypoint TOGGLE

#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0
#define     MAIN_INSTRUCTIONS 4


#define     counter r1
#define     delay 8


/////////////////////////////////////////////////////////////////////////////
// This is the toggle loop, which flips the GPIO in each iteration.
// delay loop takes 2*delay instructions
// each toggle takes 4+2*delay instructions
// 1 instruction takes 5 ns
// To achieve 5 MHz:
//   Need to toggle every 100 ns
//   100 ns = (4+2*delay) * 5 ns
//   20 = 4+2*delay
//   16 = 2*delay
//   8 = delay

// Assume delay of 8. Each toggle takes (4+16)*5 ns

TOGGLE:
    XOR     r30.b1, r30.b1, (1 << 7)                // |--- 1 instruction
    MOV     counter, delay                          // |--- 1 instructions
    MOV     r0, r0                                  // |--- 1 instruction (NOP)
DELAY:                                              //
    SUB     counter, counter, 1                     // |--- 1 instruction
    QBNE    DELAY, counter, 0                       // |--- 1 instruction

    JMP     TOGGLE                                  // |--- 1 instruction



/////////////////////////////////////////////////////////////////////////////
// Clean up. Will never be reached
END:
    CLR    r30, r30, 15            // Clear the pin
    MOV    R31.b0, PRU0_R31_VEC_VALID | SIGNUM
    HALT
/////////////////////////////////////////////////////////////////////////////
