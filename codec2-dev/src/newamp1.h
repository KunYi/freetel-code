/*---------------------------------------------------------------------------*\

  FILE........: newamp1.h
  AUTHOR......: David Rowe
  DATE CREATED: Jan 2017

  Quantisation functions for the sinusoidal coder, using "newamp1"
  algorithm that resamples variable rate L [Am} to a fixed rate K then
  VQs.

\*---------------------------------------------------------------------------*/

/*
  Copyright David Rowe 2017

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

#ifndef __NEWAMP1__
#define __NEWAMP1__

#include "codec2_fft.h"
#include "comp.h"

void interp_para(float y[], float xp[], float yp[], int np, float x[], int n);
float ftomel(float fHz);
void mel_sample_freqs_kHz(float rate_K_sample_freqs_kHz[], int K);
void resample_const_rate_f(MODEL *model, float rate_K_vec[], float rate_K_sample_freqs_kHz[], int K);
float rate_K_mbest_encode(int *indexes, float *x, float *xq, int ndim, int mbest_entries);
void post_filter_newamp1(float vec[], float sample_freq_kHz[], int K, float pf_gain);
void interp_Wo_v(float Wo_[], int voicing_[], float Wo1, float Wo2, int voicing1, int voicing2);
void resample_rate_L(MODEL *model, float rate_K_vec[], float rate_K_sample_freqs_kHz[], int K);
void determine_phase(MODEL *model, int Nfft, codec2_fft_cfg fwd_cfg, codec2_fft_cfg inv_cfg);
void newamp1_model_to_indexes(int    indexes[], 
                              MODEL *model, 
                              float  rate_K_vec[], 
                              float  rate_K_sample_freqs_kHz[], 
                              int    K,
                              float *mean,
                              float  rate_K_vec_no_mean[], 
                              float  rate_K_vec_no_mean_[]
                              );
void newamp1_indexes_to_model(float  rate_K_vec_[],  
                              float  rate_K_vec_no_mean_[],
                              float  rate_K_sample_freqs_kHz[], 
                              int    K,
                              float *mean_,
                              int    indexes[]);

#endif
