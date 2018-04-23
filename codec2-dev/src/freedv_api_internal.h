/*---------------------------------------------------------------------------*\

  FILE........: freedv_api_internal.h
  AUTHOR......: David Rowe
  DATE CREATED: August 2014

  This declares the structure freedv.  A pointer to this structure is
  returned by the FreeDV API freedv_open() function.  The pointer is used
  by the other FreeDV API functions declared in freedv_api.h.  This
  structure is intended to be internal to the FreeDV API.  The public
  functions are declared in freedv_api.h.  Changes to this structure
  are expected.  Changes (except additions) to freedv_api.h are
  discouraged.

\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2014 David Rowe

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

#ifdef __cplusplus
  extern "C" {
#endif

#ifndef __FREEDV__

#include "varicode.h"
#include "fsk.h"
#include "fmfsk.h"
#include "codec2_fdmdv.h"
#include "codec2_cohpsk.h"

struct quisk_cfFilter {        // Structure to hold the static data for FIR filters
    float * dCoefs;    // filter coefficients
    int nBuf;          // dimension of cBuf
    int nTaps;         // dimension of dSamples, cSamples, dCoefs
    int decim_index;   // index of next sample for decimation
    COMP * cSamples;   // storage for old samples
    COMP * ptcSamp;    // next available position in cSamples
    COMP * cBuf;       // auxillary buffer for interpolation
} ;

static int quisk_cfInterpDecim(COMP *, int, struct quisk_cfFilter *, int, int);
static void quisk_filt_cfInit(struct quisk_cfFilter *, float *, int);
static void quisk_filt_destroy(struct quisk_cfFilter *);
static float quiskFilt120t480[480];

struct freedv {
    int                  mode;

    /* states for various modems we support */
    
    struct CODEC2       *codec2;
    struct FDMDV        *fdmdv;
    struct COHPSK       *cohpsk;
    struct FSK          *fsk;
    struct FMFSK        *fmfsk;
    struct OFDM         *ofdm;
    struct LDPC         *ldpc;
    struct MODEM_STATS   stats;
    
    struct freedv_vhf_deframer * deframer;      // Extracts frames from VHF stream

    struct quisk_cfFilter * ptFilter7500to8000; // Filters to change to/from 7500 and 8000 sps for 700 .... 700C
    struct quisk_cfFilter * ptFilter8000to7500;

    int                  n_speech_samples;       // number of speech samples we need for each freedv_tx() call
    int                  n_nom_modem_samples;    // size of tx and most rx modem sample buffers
    int                  n_max_modem_samples;    // make your rx modem sample buffers this big
    int                  n_nat_modem_samples;    // tx modem sample block length as used by the modem before interpolation to output
                                                 // usually the same as n_nom_modem_samples, except for 700..700C
    int                  modem_sample_rate;      // ATM caller is responsible for meeting this
    int                  clip;                   // non-zero for cohpsk modem output clipping for low PAPR

    unsigned char       *packed_codec_bits;
    int                 *codec_bits;
    int                 *tx_bits;
    int                 *fdmdv_bits;
    int                 *rx_bits;
    int                  n_codec_bits;           // number of codec bits in a frame

    int                  tx_sync_bit;
    int                  smooth_symbols;

    /* test frame states -------------------------------------------------------------------------*/
    
    int                 *ptest_bits_coh;
    int                 *ptest_bits_coh_end;

    int                  test_frames;            // set this baby for 1 to tx/rx test frames to look at bit error stats
    int                  test_frames_diversity;  // 1 -> used combined carriers for error counting on 700 waveforms
    int                  test_frame_sync_state;
    int                  test_frame_sync_state_upper;  // when test_frames_diveristy==0 we need extra states for upper carriers
    int                  test_frame_count;
    int                  total_bits;
    int                  total_bit_errors;
    int                  total_bits_coded;
    int                  total_bit_errors_coded;
    int                  sz_error_pattern;

    /* optional user defined function to pass error pattern when a test frame is received */

    void                *error_pattern_callback_state;
    void (*freedv_put_error_pattern)(void *error_pattern_callback_state, short error_pattern[], int sz_error_pattern);

    int                  sync;
    int                  evenframe;
    float                snr_est;
    float                snr_squelch_thresh;
    int                  squelch_en;
    int                  nin;

    /* Varicode txt channel states ----------------------------------------------------------------------*/
    
    struct VARICODE_DEC  varicode_dec_states;
    short                tx_varicode_bits[VARICODE_MAX_BITS];
    int                  nvaricode_bits;
    int                  varicode_bit_index;

    /* interleaved LDPC OFDM states ---------------------------------------------------------------------*/

    int                  interleave_frames;          // number of OFDM modem frames in interleaver, e.g. 1,2,4,8,16
    COMP                *codeword_symbols;
    float               *codeword_amps;
    
    /* user defined function ptrs to produce and consume ASCII
      characters using aux txt channel */

    char (*freedv_get_next_tx_char)(void *callback_state);
    void (*freedv_put_next_rx_char)(void *callback_state, char c);
    void                *callback_state;
    
    /* user defined functions to produce and consume protocol bits */
    /* Protocol bits are packed MSB-first */
    void (*freedv_put_next_proto)(void *callback_state, char *proto_bits_packed);
    void (*freedv_get_next_proto)(void *callback_state, char *proto_bits_packed);
    void *proto_callback_state;
    int n_protocol_bits;
};

// FIR filter suitable for changing rates 7500 to/from 8000
// Sample 120000 Hz, pass 2700, stop 3730, ripple 0.1dB, atten 100 dB.  Stop 0.03108.
static float quiskFilt120t480[480] = { -0.000005050567303837, -0.000000267011791999, 0.000000197734700398, 0.000001038946634000,
 0.000002322193058869, 0.000004115682735322, 0.000006499942123311, 0.000009551098482930, 0.000013350669444763,
 0.000017966192635412, 0.000023463361155584, 0.000029885221425020, 0.000037271082107518, 0.000045630720487935,
 0.000054970017069384, 0.000065233162392019, 0.000076360900545177, 0.000088271373315159, 0.000100818605854714,
 0.000113853476544409, 0.000127174196746337, 0.000140558396336177, 0.000153744508371709, 0.000166450784469067,
 0.000178368313347299, 0.000189176709991702, 0.000198541881389953, 0.000206128795372885, 0.000211604878787747,
 0.000214655997661182, 0.000214994859281552, 0.000212358734245594, 0.000206539880117977, 0.000197379393194548,
 0.000184780318878738, 0.000168719942655099, 0.000149250512353807, 0.000126511346757621, 0.000100726393185629,
 0.000072210925236429, 0.000041365841965015, 0.000008680571408025, -0.000025277165852799, -0.000059865389594949,
-0.000094384355854646, -0.000128080670195777, -0.000160170174848483, -0.000189854272533545, -0.000216333899003825,
-0.000238836419299503, -0.000256632149501508, -0.000269058714331757, -0.000275541485292432, -0.000275614059005332,
-0.000268937472718753, -0.000255317038867589, -0.000234717772155001, -0.000207273956099563, -0.000173297342436372,
-0.000133280012107173, -0.000087895370243821, -0.000037986085678081, 0.000015440388211825, 0.000071232572821451,
 0.000128114399130489, 0.000184710477990398, 0.000239577162514028, 0.000291234779803098, 0.000338204791740229,
 0.000379047713684221, 0.000412403761615261, 0.000437031818051652, 0.000451848709179591, 0.000455966225408344,
 0.000448726371643413, 0.000429729020814434, 0.000398857326863837, 0.000356297600912998, 0.000302547334727027,
 0.000238422248479072, 0.000165048886226905, 0.000083853091464077, -0.000003462782744354, -0.000094949813106744,
-0.000188451833293202, -0.000281651282503015, -0.000372121907291206, -0.000457387566635848, -0.000534985542936898,
-0.000602532044011899, -0.000657788245032425, -0.000698728981427767, -0.000723604675185869, -0.000731002305621048,
-0.000719899536922384, -0.000689709694056092, -0.000640319946685634, -0.000572115873292030, -0.000485996080304965,
-0.000383371840261246, -0.000266155252511831, -0.000136731311264191, 0.000002082667095075, 0.000147092077716480,
 0.000294790953130229, 0.000441441918072383, 0.000583164190168290, 0.000716029226064227, 0.000836164238172957,
 0.000939856052624227, 0.001023657909064450, 0.001084492755093968, 0.001119751426837743, 0.001127383039339373,
 0.001105974243787613, 0.001054815583369999, 0.000973950761085690, 0.000864209315714227, 0.000727219011746881,
 0.000565398080608305, 0.000381924396468366, 0.000180685902835315, -0.000033793183292569, -0.000256444114966522,
-0.000481764526566339, -0.000703946352348464, -0.000917016099829735, -0.001114986581270253, -0.001292014799874503,
-0.001442563411804926, -0.001561559957317790, -0.001644551048567398, -0.001687846581475964, -0.001688649703502788,
-0.001645167889846890, -0.001556702802350076, -0.001423714708648073, -0.001247857669697092, -0.001031986722557201,
-0.000780131048444402, -0.000497436825078657, -0.000190077210351809, 0.000134868279325909, 0.000469563533327739,
 0.000805591531546815, 0.001134152328775355, 0.001446279849797673, 0.001733071409562941, 0.001985924997799762,
 0.002196778054604388, 0.002358342626407065, 0.002464328098407475, 0.002509648218888532, 0.002490604086803692,
 0.002405037734357425, 0.002252452724297770, 0.002034094661603120, 0.001752990365583534, 0.001413941154886139,
 0.001023470495638453, 0.000589723521647734, 0.000122320866350319, -0.000367832138027160, -0.000868777013398284,
-0.001367771151677059, -0.001851587344265625, -0.002306838088978190, -0.002720317947026380, -0.003079353614002113,
-0.003372155891804708, -0.003588162376578369, -0.003718362558663737, -0.003755596511143005, -0.003694818131674599,
-0.003533315298404129, -0.003270878754553819, -0.002909914962857412, -0.002455496391464944, -0.001915346645364514,
-0.001299757227227888, -0.000621437066532776, 0.000104706515738248, 0.000861849931067767, 0.001631595707499856,
 0.002394368911341672, 0.003129858565588139, 0.003817496679992245, 0.004436963307209760, 0.004968707287606522,
 0.005394469536085115, 0.005697797543539088, 0.005864537618023589, 0.005883292537600076, 0.005745832319314692,
 0.005447447099071761, 0.004987231255534477, 0.004368289529377007, 0.003597859022418248, 0.002687338851256991,
 0.001652226293162047, 0.000511956075882180, -0.000710356149138656, -0.001988263330091648, -0.003292424566049982,
-0.004591123342747130, -0.005850857852106148, -0.007036991266043732, -0.008114450164977267, -0.009048456200082230,
-0.009805276478965942, -0.010352975302354198, -0.010662152577592631, -0.010706650669328861, -0.010464214075017983,
-0.009917087295446811, -0.009052534679222271, -0.007863270920348924, -0.006347789704693751, -0.004510582323649121,
-0.002362238055733795, 0.000080576968834213, 0.002795265196543707, 0.005753566158586979, 0.008921944932552510,
 0.012262093950265378, 0.015731539846483594, 0.019284344624007944, 0.022871886384520687, 0.026443706729191677,
 0.029948406200633094, 0.033334570666910354, 0.036551709955124537, 0.039551189200810140, 0.042287133974308874,
 0.044717290029466283, 0.046803820535016104, 0.048514022996355009, 0.049820951883635139, 0.050703932928426454,
 0.051148959210315710, 0.051148959210315710, 0.050703932928426454, 0.049820951883635139, 0.048514022996355009,
 0.046803820535016104, 0.044717290029466283, 0.042287133974308874, 0.039551189200810140, 0.036551709955124537,
 0.033334570666910354, 0.029948406200633094, 0.026443706729191677, 0.022871886384520687, 0.019284344624007944,
 0.015731539846483594, 0.012262093950265378, 0.008921944932552510, 0.005753566158586979, 0.002795265196543707,
 0.000080576968834213, -0.002362238055733795, -0.004510582323649121, -0.006347789704693751, -0.007863270920348924,
-0.009052534679222271, -0.009917087295446811, -0.010464214075017983, -0.010706650669328861, -0.010662152577592631,
-0.010352975302354198, -0.009805276478965942, -0.009048456200082230, -0.008114450164977267, -0.007036991266043732,
-0.005850857852106148, -0.004591123342747130, -0.003292424566049982, -0.001988263330091648, -0.000710356149138656,
 0.000511956075882180, 0.001652226293162047, 0.002687338851256991, 0.003597859022418248, 0.004368289529377007,
 0.004987231255534477, 0.005447447099071761, 0.005745832319314692, 0.005883292537600076, 0.005864537618023589,
 0.005697797543539088, 0.005394469536085115, 0.004968707287606522, 0.004436963307209760, 0.003817496679992245,
 0.003129858565588139, 0.002394368911341672, 0.001631595707499856, 0.000861849931067767, 0.000104706515738248,
-0.000621437066532776, -0.001299757227227888, -0.001915346645364514, -0.002455496391464944, -0.002909914962857412,
-0.003270878754553819, -0.003533315298404129, -0.003694818131674599, -0.003755596511143005, -0.003718362558663737,
-0.003588162376578369, -0.003372155891804708, -0.003079353614002113, -0.002720317947026380, -0.002306838088978190,
-0.001851587344265625, -0.001367771151677059, -0.000868777013398284, -0.000367832138027160, 0.000122320866350319,
 0.000589723521647734, 0.001023470495638453, 0.001413941154886139, 0.001752990365583534, 0.002034094661603120,
 0.002252452724297770, 0.002405037734357425, 0.002490604086803692, 0.002509648218888532, 0.002464328098407475,
 0.002358342626407065, 0.002196778054604388, 0.001985924997799762, 0.001733071409562941, 0.001446279849797673,
 0.001134152328775355, 0.000805591531546815, 0.000469563533327739, 0.000134868279325909, -0.000190077210351809,
-0.000497436825078657, -0.000780131048444402, -0.001031986722557201, -0.001247857669697092, -0.001423714708648073,
-0.001556702802350076, -0.001645167889846890, -0.001688649703502788, -0.001687846581475964, -0.001644551048567398,
-0.001561559957317790, -0.001442563411804926, -0.001292014799874503, -0.001114986581270253, -0.000917016099829735,
-0.000703946352348464, -0.000481764526566339, -0.000256444114966522, -0.000033793183292569, 0.000180685902835315,
 0.000381924396468366, 0.000565398080608305, 0.000727219011746881, 0.000864209315714227, 0.000973950761085690,
 0.001054815583369999, 0.001105974243787613, 0.001127383039339373, 0.001119751426837743, 0.001084492755093968,
 0.001023657909064450, 0.000939856052624227, 0.000836164238172957, 0.000716029226064227, 0.000583164190168290,
 0.000441441918072383, 0.000294790953130229, 0.000147092077716480, 0.000002082667095075, -0.000136731311264191,
-0.000266155252511831, -0.000383371840261246, -0.000485996080304965, -0.000572115873292030, -0.000640319946685634,
-0.000689709694056092, -0.000719899536922384, -0.000731002305621048, -0.000723604675185869, -0.000698728981427767,
-0.000657788245032425, -0.000602532044011899, -0.000534985542936898, -0.000457387566635848, -0.000372121907291206,
-0.000281651282503015, -0.000188451833293202, -0.000094949813106744, -0.000003462782744354, 0.000083853091464077,
 0.000165048886226905, 0.000238422248479072, 0.000302547334727027, 0.000356297600912998, 0.000398857326863837,
 0.000429729020814434, 0.000448726371643413, 0.000455966225408344, 0.000451848709179591, 0.000437031818051652,
 0.000412403761615261, 0.000379047713684221, 0.000338204791740229, 0.000291234779803098, 0.000239577162514028,
 0.000184710477990398, 0.000128114399130489, 0.000071232572821451, 0.000015440388211825, -0.000037986085678081,
-0.000087895370243821, -0.000133280012107173, -0.000173297342436372, -0.000207273956099563, -0.000234717772155001,
-0.000255317038867589, -0.000268937472718753, -0.000275614059005332, -0.000275541485292432, -0.000269058714331757,
-0.000256632149501508, -0.000238836419299503, -0.000216333899003825, -0.000189854272533545, -0.000160170174848483,
-0.000128080670195777, -0.000094384355854646, -0.000059865389594949, -0.000025277165852799, 0.000008680571408025,
 0.000041365841965015, 0.000072210925236429, 0.000100726393185629, 0.000126511346757621, 0.000149250512353807,
 0.000168719942655099, 0.000184780318878738, 0.000197379393194548, 0.000206539880117977, 0.000212358734245594,
 0.000214994859281552, 0.000214655997661182, 0.000211604878787747, 0.000206128795372885, 0.000198541881389953,
 0.000189176709991702, 0.000178368313347299, 0.000166450784469067, 0.000153744508371709, 0.000140558396336177,
 0.000127174196746337, 0.000113853476544409, 0.000100818605854714, 0.000088271373315159, 0.000076360900545177,
 0.000065233162392019, 0.000054970017069384, 0.000045630720487935, 0.000037271082107518, 0.000029885221425020,
 0.000023463361155584, 0.000017966192635412, 0.000013350669444763, 0.000009551098482930, 0.000006499942123311,
 0.000004115682735322, 0.000002322193058869, 0.000001038946634000, 0.000000197734700398, -0.000000267011791999,
-0.000005050567303837 };
#endif

#ifdef __cplusplus
}
#endif
