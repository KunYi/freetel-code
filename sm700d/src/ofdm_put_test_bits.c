/*---------------------------------------------------------------------------*\

  FILE........: ofdm_put_test_bits.c
  AUTHOR......: David Rowe
  DATE CREATED: Mar 2018

  Slurps up a stream of test bits generated by ofdm_get_test_bits, useful for
  testing ofdm_mod and ofdm_demod.

\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2018 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "codec2_ofdm.h"
#include "ofdm_internal.h"
#include "test_bits_ofdm.h"

#define LOG_FRAMES 100
#define NDISCARD   20

static struct OFDM *ofdm;
static struct OFDM_CONFIG *ofdm_config;

int opt_exists(char *argv[], int argc, char opt[]) {
    int i;
    for (i=0; i<argc; i++) {
        if (strcmp(argv[i], opt) == 0) {
            return i;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    FILE         *fin;
    int           i, f, Nerrs, Terrs, Tbits, Terrs2, Tbits2, verbose;
    float         aber;

    if (argc < 2) {
	fprintf(stderr, "\n");
	fprintf(stderr, "usage: %s InputOneCharPerBitFile [-v]\n", argv[0]);
	fprintf(stderr, "\n");
	exit(1);
    }

    if (strcmp(argv[1], "-") == 0)
        fin = stdin;
    else if ( (fin = fopen(argv[1],"rb")) == NULL ) {
	fprintf(stderr, "Error opening input file: %s: %s.\n",
         argv[1], strerror(errno));
	exit(1);
    }

    verbose = 0;
    if (opt_exists(argv, argc, "-v")) {
        verbose = 1;
    }

    if ((ofdm_config = (struct OFDM_CONFIG *) malloc(sizeof (struct OFDM_CONFIG))) == NULL) {
	printf("Out of Memory");
	exit(1);
    }

    ofdm_config->centre = 1500.0;              /* Centre Audio Frequency */
    ofdm_config->fs = 8000.0;                  /* Sample Frequency */
    ofdm_config->ts = 0.018;                   /* symbol duration */
    ofdm_config->rs = (1.0 / ofdm_config->ts); /* Symbol Rate */
    ofdm_config->nc = 17;                      /* Number of carriers */
    ofdm_config->tcp = .002;                   /* Cyclic Prefix duration */
    ofdm_config->ns = 8;                       /* Number of Symbol frames */
    ofdm_config->ofdm_timing_mx_thresh = 0.30;
    ofdm_config->bps = 2;                      /* Bits per Symbol */
    ofdm_config->txtbits = 4;                  /* number of auxiliary data bits */
    ofdm_config->state_str = 16;               /* state string length */
    ofdm_config->ftwindowwidth = 11;

    ofdm = ofdm_create(ofdm_config);
    assert(ofdm != NULL);

    int Nbitsperframe = ofdm_get_bits_per_frame();
    char rx_bits[Nbitsperframe];
  
    f = Terrs = Tbits = Terrs2 = Tbits2 = 0;
    while (fread(rx_bits, sizeof(char), Nbitsperframe, fin) == Nbitsperframe) {
        f++;
        
        Nerrs = 0;
        for(i=0; i<Nbitsperframe; i++) {
            if (test_bits_ofdm[i] != rx_bits[i]) {
                Nerrs++;
            }
        }
        aber = (float)Nerrs/Nbitsperframe;

        Terrs += Nerrs;
        Tbits += Nbitsperframe;

        if (f >= NDISCARD) {
            Terrs2 += Nerrs;
            Tbits2 += Nbitsperframe;
        }
        
        if (verbose) {
            printf("f: %d Nerrs: %d aber: %3.2f\n", f, Nerrs, aber);
        }
        
        if (fin == stdin) fflush(stdin);
    }

    fclose(fin);

    fprintf(stderr, "BER..: %5.4f Tbits: %5d Terrs: %5d\n", (float)Terrs/Tbits, Tbits, Terrs);

    if (Tbits2 != 0) {
        fprintf(stderr, "BER2.: %5.4f Tbits: %5d Terrs: %5d\n", (float)Terrs2/Tbits2, Tbits2, Terrs2);
    }

    ofdm_destroy(ofdm);
    free(ofdm_config);

    return 0;
}

