/*---------------------------------------------------------------------------*\
                                                               
  FILE........: sinenc.c                                    
  AUTHOR......: David Rowe                                  
  DATE CREATED: 20/2/95                                       
                                                                             
  Sinusoidal speech encoder program using external (Matlab) pitch estimator.
                                                                             
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2009 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "dump.h"
#include "sine.h"

/*---------------------------------------------------------------------------*\
                                                                             
 switch_present()                                                            
                                                                             
 Searches the command line arguments for a "switch".  If the switch is       
 found, returns the command line argument where it ws found, else returns    
 NULL.                                                                       
                                                                             
\*---------------------------------------------------------------------------*/

int switch_present(sw,argc,argv)
  char sw[];     /* switch in string form */
  int argc;      /* number of command line arguments */
  char *argv[];  /* array of command line arguments in string form */
{
  int i;       /* loop variable */

  for(i=1; i<argc; i++)
    if (!strcmp(sw,argv[i]))
      return(i);

  return 0;
}

/*---------------------------------------------------------------------------*\
                                                                            
				    MAIN                                          
                                                                             
\*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  FILE  *fin;		/* input speech sample file */
  FILE  *fmodel;	/* output file of model parameters */
  FILE  *fp;		/* input text file containing pitch estimates */
  short  buf[N];	/* input speech sample buffer */
  float  Sn[M];	        /* float input speech samples */
  COMP   Sw[FFT_ENC];	/* DFT of Sn[] */
  float  w[M];	        /* time domain hamming window */
  COMP   W[FFT_ENC];	/* DFT of w[] */
  MODEL  model;
  int    length;	/* number of frames to process */
  float  pitch;		/* current pitch estimate from external pitch file */
  int    i;		/* loop variable */
  FILE  *fref;		/* optional output file with refined pitch estimate */
  int    arg;
  int    dump;
  int    frames;

  if (argc < 5) {
    printf("usage: sinenc InputFile ModelFile Frames PitchFile\n");
    exit(1);
  }

  /* Interpret command line arguments -------------------------------------*/

  if ((fin = fopen(argv[1],"rb")) == NULL) {
    printf("Error opening input file: %s\n",argv[1]);
    exit(1);
  }

  if ((fmodel = fopen(argv[2],"wb")) == NULL) {
    printf("Error opening output model file: %s\n",argv[2]);
    exit(1);
  }

  length = atoi(argv[3]);

  if ((fp = fopen(argv[4],"rt")) == NULL) {
    printf("Error opening input pitch file: %s\n",argv[4]);
    exit(1);
  }

  dump = switch_present("--dump",argc,argv);
  if (dump) 
      dump_on(argv[dump+1]);

  if ((arg = switch_present("--ref",argc,argv))) {
    if ((fref = fopen(argv[arg+1],"wt")) == NULL) {
      printf("Error opening output pitch refinement file: %s\n",argv[5]);
      exit(1);
    }
  }
  else
    fref = NULL;

  /* Initialise sample buffer memories to stop divide by zero errors
     and zero energy frames at the start of simulation */

  for(i=0; i<M; i++)
    Sn[i] = 1.0;

  make_analysis_window(w, W);
 
  /* Main loop ------------------------------------------------------------*/

  frames = 0;
  while((fread(buf,sizeof(short),N,fin) == N) && (frames != length)) {
    frames++;

    /* Update input speech buffers */

    for(i=0; i<M-N; i++)
      Sn[i] = Sn[i+N];
    for(i=0; i<N; i++)
      Sn[i+M-N] = buf[i];

    /* Estimate pitch */

    fscanf(fp,"%f\n",&pitch);

    /* construct analysis window */

    model.Wo = TWO_PI/pitch;

    /* estimate and model parameters */

    dft_speech(Sw, Sn, w); 
    two_stage_pitch_refinement(&model, Sw);
    estimate_amplitudes(&model, Sw, W);
    dump_Sn(Sn); dump_Sw(Sw); dump_model(&model);

    /* save model parameters */

    if (fref != NULL && frames > 2)
      fprintf(fref,"%f\n",model.Wo);
    fwrite(&model,sizeof(model),1,fmodel);
    printf("frame: %d\r",frames);
  }

  /* close files and exit */

  if (fref != NULL) fclose(fref);
  fclose(fin);
  fclose(fmodel);

  if (dump)
      dump_off();

  return 0;
}

