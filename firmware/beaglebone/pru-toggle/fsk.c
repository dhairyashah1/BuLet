/***
   Copyright (c) 2014 dhenke@mythopoeic.org
   This is free software -- see COPYING for details.

   example -- demonstrate simple use of AM335x PRU_ICSS by waiting 5s

   usage: sudo ./example

   Runs a very simple PRU program (expected to be in a file example.bin
   in the current working directory) then waits for this program to assert
   PRU_EVTOUT_0. When it does so, cleans up and exits.

   The PRU program simply delays for five seconds. (So, if you run this
   and it takes much more or less time than that to exit, you know something
   is amiss.)
***/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include <math.h>

#define PRU_NUM 0 /* which of the two PRUs are we using? */

static void *pru0DataMem;
static void *pru1DataMem;


#define DELAY(ns) ((ns/5-6)/2)

/*** pru_setup() -- initialize PRU and interrupt handler

Initializes the PRU specified by PRU_NUM and sets up PRU_EVTOUT_0 handler.

Returns 0 on success, non-0 on error.
***/
static int pru_setup() {
   int rtn;
   tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;

   /* initialize PRU */
   if((rtn = prussdrv_init()) != 0) {
      fprintf(stderr, "prussdrv_init() failed\n");
      return rtn;
   }

   /* open the interrupt */
   if((rtn = prussdrv_open(PRU_EVTOUT_0)) != 0) {
      fprintf(stderr, "prussdrv_open() failed\n");
      return rtn;
   }

   /* initialize interrupt */
   if((rtn = prussdrv_pruintc_init(&intc)) != 0) {
      fprintf(stderr, "prussdrv_pruintc_init() failed\n");
      return rtn;
   }

   if((rtn = prussdrv_pru_reset(PRU_NUM)) != 0) {
      fprintf(stderr, "prussdrv_pru_reset() failed\n");
      return rtn;
   }

   if ((rtn = prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pru0DataMem)) != 0) {
     fprintf(stderr, "prussdrv_map_prumem() failed\n");
     return rtn;
   }

   if ((rtn = prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, &pru1DataMem)) != 0) {
     fprintf(stderr, "prussdrv_map_prumem() failed\n");
     return rtn;
   }

   return rtn;
}

/*** pru_cleanup() -- halt PRU and release driver

Performs all necessary de-initialization tasks for the prussdrv library.

Returns 0 on success, non-0 on error.
***/
static int pru_cleanup(void) {
   int rtn = 0;

   /* clear the event (if asserted) */
   if(prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT)) {
      fprintf(stderr, "prussdrv_pru_clear_event() failed\n");
      rtn = -1;
   }

   /* halt and disable the PRU (if running) */
   if((rtn = prussdrv_pru_disable(PRU_NUM)) != 0) {
      fprintf(stderr, "prussdrv_pru_disable() failed\n");
      rtn = -1;
   }

   /* release the PRU clocks and disable prussdrv module */
   if((rtn = prussdrv_exit()) != 0) {
      fprintf(stderr, "prussdrv_exit() failed\n");
      rtn = -1;
   }

   return rtn;
}

/* Adapted from https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform#Implementation */
double generateGaussianNoise(double mu, double sigma)
{
    const double epsilon = 0.00000000001; //std::numeric_limits<double>::min();
    const double two_pi = 2.0*3.14159265358979323846;

    static double z0, z1;
    static int generate;
    generate = !generate;

    if (!generate)
       return z1 * sigma + mu;

    double u1, u2;
    do
     {
       u1 = rand() * (1.0 / RAND_MAX);
       u2 = rand() * (1.0 / RAND_MAX);
     }
    while ( u1 <= epsilon );

    z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
    z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
    return z0 * sigma + mu;
}

int8_t quantize(double val) {
    return ((int8_t) round(val/10.)) * 10;
}


int main(int argc, char **argv) {
   int rtn;

   /* prussdrv_init() will segfault if called with EUID != 0 */ 
   if(geteuid()) {
      fprintf(stderr, "%s must be run as root to use prussdrv\n", argv[0]);
      return -1;
   }

   if (argc < 2) {
      fprintf(stderr, "Usage: %s TXFILE [JITTER_STDDEV]\n", argv[0]);
      return -1;
   }

   /* initialize the library, PRU and interrupt; launch our PRU program */
   if(pru_setup()) {
      pru_cleanup();
      return -1;
   }

   uint16_t buf[8192];
   FILE *fd = fopen(argv[1], "rb");
   size_t n = fread(buf, 1, 2*8192, fd);
   printf("Read %d bytes.\n", n);

   double jitter_std = 0.0;
   if (argc == 3) {
      jitter_std = atof(argv[2]);
      printf("Jitter std deviation set to %f\n", jitter_std);
   }

   uint8_t *pruMem8 = (uint8_t *) pru0DataMem;
   int i;
   int8_t jitter = 0;
   for (i=0;i<n/2;i+=2) {
      pruMem8[i] = buf[i+1] & 0xff;

      if (jitter_std != 0) {
         jitter = quantize(generateGaussianNoise(0.0, jitter_std));
      }
      if (buf[i]+jitter < 80) {
         buf[i] = 80;
      } else {
         buf[i] = buf[i]+jitter;
      }

//      printf("jittered to %hu (%hhd)\n", buf[i], jitter);
      pruMem8[i+1] = DELAY(buf[i]) & 0xff;
//      printf("%d: %hu iterations with (6+2*%hu)*5 instructions (%hu ns)\n",
//             i/2, pruMem8[i], pruMem8[i+1], buf[i]);
   }
   pruMem8[i] = 0;

   /* load and run the PRU program */
   if((rtn = prussdrv_exec_program(PRU_NUM, "fsk.bin")) < 0) {
      fprintf(stderr, "prussdrv_exec_program() failed\n");
      return rtn;
   }

   /* wait for PRU to assert the interrupt to indicate completion */
   printf("waiting for interrupt from PRU0...\n");

   /* The prussdrv_pru_wait_event() function returns the number of times
      the event has taken place, as an unsigned int. There is no out-of-
      band value to indicate error (and it can wrap around to 0 if you
      run the program just a whole lot of times). */
   rtn = prussdrv_pru_wait_event(PRU_EVTOUT_0);

   printf("PRU program completed, event number %d\n", rtn);

   /* clear the event, disable the PRU and let the library clean up */
   return pru_cleanup();
}
