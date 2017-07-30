% newamp1_fbf.m
%
% Copyright David Rowe 2016
% This program is distributed under the terms of the GNU General Public License 
% Version 2
%
% Interactive Octave script to explore frame by frame operation of new amplitude
% modelling model.
%
% Usage:
%   Make sure codec2-dev is compiled with the -DDUMP option - see README for
%    instructions.
%   ~/codec2-dev/build_linux/src$ ./c2sim ../../raw/hts1a.raw --dump hts1a
%   $ cd ~/codec2-dev/octave
%   octave:14> newamp1_fbf("../build_linux/src/hts1a",50)


function newamp1_fbf(samname, f=73, varargin)
  more off;

  newamp;
  melvq;

  Fs = 8000; rate_K_sample_freqs_kHz = [0.1:0.1:4]; K = length(rate_K_sample_freqs_kHz);
  quant_en = 0; vq_search = "gain";
  mask_en = 0;
  nvec = 0;
  weight_en = quant_en = 0;

  % optional VQ

  ind = arg_exists(varargin, "vq");
  if ind
    nvec++;
    vq_filename = varargin{ind+2};
    x = load(vq_filename); vq = x.vq;
    [vq_rows vq_cols] = size(vq); vq_st = varargin{ind+1}; vq_en = vq_st + vq_cols - 1;
  end
  
  % different vq search algorithms

  ind = arg_exists(varargin, "vq_search");
  if ind
    vq_search = varargin{ind+1};
  end

  fit_order = 0;
  ind = arg_exists(varargin, "noslope");
  if ind
    fit_order = 1;
  end

  if quant_en
    printf("quant_en: %d vq_filename: %s vq_st: %d vq_en: %d vq_search: %s\n", 
           quant_en, vq_filename, vq_st, vq_en, vq_search);
  end

  % load up text files dumped from c2sim ---------------------------------------

  sn_name = strcat(samname,"_sn.txt");
  Sn = load(sn_name);
  sw_name = strcat(samname,"_sw.txt");
  Sw = load(sw_name);
  model_name = strcat(samname,"_model.txt");
  model = load(model_name);
  [frames tmp] = size(model);

  % Keyboard loop --------------------------------------------------------------

  k = ' ';
  do 
    s = [ Sn(2*f-1,:) Sn(2*f,:) ];
    figure(1); clf; plot(s); axis([1 length(s) -20000 20000]);

    Wo = model(f,1); L = model(f,2); Am = model(f,3:(L+2)); AmdB = 20*log10(Am);
    Am_freqs_kHz = (1:L)*Wo*4/pi;

    % remove constant gain term

    rate_K_vec = resample_const_rate_f(model(f,:), rate_K_sample_freqs_kHz, Fs);
    if fit_order == 0
      slope = 0; meanf = mean(rate_K_vec); 
      rate_K_vec_fit = rate_K_vec - meanf;
    end

    % plots ----------------------------------
  
    figure(2); clf;
    l = sprintf(";rate %d AmdB;g+-", L);
    plot((1:L)*Wo*4000/pi, AmdB, l);
    axis([1 4000 -20 80]);
    hold on;
    plot(rate_K_sample_freqs_kHz*1000, rate_K_vec, ";rate K;b+-");

    % default to the ideal
    
    rate_K_vec_ = rate_K_vec_fit; 

    if mask_en && nvec
      % experimental masking stuff that I can't seem to get to work
      maskdB = determine_mask(rate_K_vec, rate_K_sample_freqs_kHz, rate_K_sample_freqs_kHz, bark_model=1);
      plot(rate_K_sample_freqs_kHz*1000, maskdB, ";mask dB;c+-");
    end

    if nvec
      if mask_en
        target = rate_K_vec;
        mask_thresh = 3;
        ind = find (maskdB - target > mask_thresh);
        target(ind) = maskdB(ind) - mask_thresh;
        plot(rate_K_sample_freqs_kHz*1000, target, ";target;m+-");
        target = target(vq_st:vq_en) - b;
      else
        target = rate_K_vec_fit(vq_st:vq_en);
      end

      weights = ones(1, vq_en - vq_st + 1);
      if weight_en

        % generate weighting.  if min is 20dB and max 40dB, weight at min
        % is 1 and max 2, same for -10 to 10.  So gradient = 0.05, we only
        % haveto calculate y intercept

        gradient = 0.05; yint = 1 - gradient*min(target);
        weights = gradient*target + yint;
      end
    end
    
    if nvec
      
      if strcmp(vq_search, "mse")
        [idx contrib errors test_ g mg sl] = vq_search_mse(vq, target);
        rate_K_surface_fit_(f, vq_st:vq_en) = contrib;
      end

      if strcmp(vq_search, "gain")
        [idx contrib errors test_ g mg sl] = vq_search_gain(vq, target, weights);
      end

      if strcmp(vq_search, "slope")
        [idx contrib errors test_ g mg sl] = vq_search_slope1(vq, target, weights);
        rate_K_surface_fit_(f, vq_st:vq_en) = contrib;
        % printf("g: %3.2f mg: %3.2f sl: %3.2f\n", g(idx), mg(idx), sl(idx));
      end

      if strcmp(vq_search, "para")
        printf("\n");
        [idx contrib errors b] = vq_search_para(vq, target);
        if quant_en
          % recalc gain using some index
          g = (sum(target(1:vq_cols-10)) - sum(mg*vq(idx,1:vq_cols-10)))/length(target);
          contrib = mg*vq(idx,:) + g;
          contrib(vq_cols-4:vq_cols) += -3:-6:-30;
        end
        rate_K_surface_fit_(f, vq_st:vq_en) = contrib;
        % printf("g: %3.2f mg: %3.2f sl: %3.2f\n", g(idx), mg(idx), sl(idx));
      end

      rate_K_vec_(vq_st:vq_en) = contrib;

      plot(rate_K_sample_freqs_kHz(vq_st:vq_en)*1000, contrib, 'm+-');
      l = sprintf(";diff vq sd = %3.2f;k+-", std(target - contrib));
      plot(rate_K_sample_freqs_kHz(vq_st:vq_en)*1000, target - contrib, l);
    end

    % And .... back to rate L
    
    rate_K_vec_ += meanf;
    [model_ AmdB_] = resample_rate_L(model(f,:), rate_K_vec_, rate_K_sample_freqs_kHz, Fs);
    AmdB_ = AmdB_(1:L);
    sdL = std(abs(AmdB - AmdB_));

    plot((1:L)*Wo*4000/pi, AmdB_,";AmdB bar;r+-");
    if nvec == 0
      l = sprintf(";errorx10 sd %3.2f dB;bk+-", sdL);
      plot((1:L)*Wo*4000/pi, 10*(AmdB - AmdB_), l);
    end
    hold off;

    if weight_en
      figure(3); clf;
      subplot(211);
      plot((1:L)*Wo*4000/pi, AmdB,";AmdB;g+-");
      axis([1 4000 -20 80]);
      hold on;
      plot(rate_K_sample_freqs_kHz*1000, rate_K_vec, ";rate K;b+-");
      hold off;
      subplot(212);
      plot(rate_K_sample_freqs_kHz(vq_st:vq_en)*1000, weights);
      axis([1 4000 0 8]);
    end

    % interactive menu ------------------------------------------

    printf("\rframe: %d  menu: n-next  b-back  q-quit  w-quant[%d]", f, quant_en);
    fflush(stdout);
    k = kbhit();

    if k == 'w'
      quant_en++;
      if quant_en == 2
        quant_en = 0;
      end
    endif
    if k == 'n'
      f = f + 1;
    endif
    if k == 'b'
      f = f - 1;
    endif
    if k == 'o'
      fit_order++;
      if fit_order == 2
        fit_order = 0;
      end
    endif
  until (k == 'q')
  printf("\n");

endfunction

 
function ind = arg_exists(v, str) 
   ind = 0;
   for i=1:length(v)
      if strcmp(v{i}, str)
        ind = i;
      end     
    end
endfunction


