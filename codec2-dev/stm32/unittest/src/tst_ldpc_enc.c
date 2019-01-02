/*
  FILE...: ldpc_enc.c
  AUTHOR.: Bill Cowley, David Rowe
  CREATED: Sep 2016

  STM32 Version: Aug 2018 - Don Reid
*/

/* This is a unit test implementation of the LDPC encode function.
 *
 * Typical run:

    ofdm_gen_test_bits stm_in.raw 6 --rand --ldpc

    ldpc_enc stm_in.raw ref_out.raw --code HRA_112_112

    <Load stm32 and run>

    cmp -l ref_out.raw stm_out.raw

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mpdecode_core.h"

#include "semihosting.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "machdep.h"

/* generated by ldpc_fsk_lib.m:ldpc_decode() */

#include "HRA_112_112.h"

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
    unsigned char ibits[HRA_112_112_NUMBERROWSHCOLS];
    unsigned char pbits[HRA_112_112_NUMBERPARITYBITS];
    FILE         *fin, *fout;
    struct LDPC   ldpc;

    semihosting_init();

    printf("LDPC encode test and profile\n");

    PROFILE_VAR(ldpc_encode);

    machdep_profile_init();

    /* set up LDPC code from include file constants */
    /* short rate 1/2 code for FreeDV HF digital voice */
    ldpc.CodeLength = HRA_112_112_CODELENGTH;
    ldpc.NumberParityBits = HRA_112_112_NUMBERPARITYBITS;
    ldpc.NumberRowsHcols = HRA_112_112_NUMBERROWSHCOLS;
    ldpc.max_row_weight = HRA_112_112_MAX_ROW_WEIGHT;
    ldpc.max_col_weight = HRA_112_112_MAX_COL_WEIGHT;
    ldpc.H_rows = (uint16_t *)HRA_112_112_H_rows;
    ldpc.H_cols = (uint16_t *)HRA_112_112_H_cols;

    fin = fopen("stm_in.raw", "rb");
    if (fin == NULL) {
        printf("Error opening input file\n");
        exit(1);
    }

    fout = fopen("stm_out.raw", "wb");
    if (fout == NULL) {
        printf("Error opening output file\n");
        exit(1);
    }

    while (fread(ibits, sizeof(char), ldpc.NumberParityBits, fin) == 
    		ldpc.NumberParityBits) {

        PROFILE_SAMPLE(ldpc_encode);
        encode(&ldpc, ibits, pbits);
        PROFILE_SAMPLE_AND_LOG2(ldpc_encode, "  ldpc_encode");

        fwrite(ibits, sizeof(char), ldpc.NumberRowsHcols, fout);
        fwrite(pbits, sizeof(char), ldpc.NumberParityBits, fout);
    }

    fclose(fin);
    fclose(fout);
    
    fflush(stdout);
    stdout = freopen("stm_profile", "w", stdout);
    machdep_profile_print_logged_samples();

    fclose(stdout);
    fclose(stderr);

    return 0;
}
