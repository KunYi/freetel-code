/// \file platform/linux/audio_in_alsa.cpp
/// ALSA audio input device driver, for use on Linux.
/// There is at least one other operating system that supports an ALSA-like
/// interface, Nucleus, but at this writing (early 2014) this driver is
/// untested on that system.
///
/// \copyright Copyright (C) 2013-2014 Algoram. See the LICENSE file.
///

#include "drivers.h"
#include "alsa.h"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

namespace FreeDV {
  std::ostream & ALSAEnumerate(std::ostream & stream, snd_pcm_stream_t mode);

  /// Audio input "ALSA", Uses the Linux ALSA Audio API.
  ///
  class AudioInALSA : public AudioInput {
  private:
    static const int	overlong_delay = AudioFrameSamples * 4;

    snd_pcm_t *		handle;
    char * const	parameters;
    bool		started;

    // Copy constructor and operator=() disabled.
    AudioInALSA(const AudioInALSA &);
    AudioInALSA & operator=(const AudioInALSA &);

    NORETURN void
    do_throw(const int error, const char * message = 0);
  public:

	/// Instantiate the audio input.
  		AudioInALSA(const char * parameters);
		~AudioInALSA();

        /// Return file descriptors for poll()
 	/// \param array The address of an array that will be written
	/// with a sequence of file descriptors.
        /// \param space The maximum number of file descriptors that may be
        /// stored in the array.
        /// \return The number of file descriptors written to the array.
	virtual int
		poll_fds(PollType * array, int space);

	/// Return the number of audio samples the device can provide in
	/// a read without blocking.
        virtual std::size_t
		ready();

        /// Read audio from the "short" type.
	virtual std::size_t
		read16(std::int16_t * array, std::size_t length);

        /// Start the audio device.
        /// Input devices: start digitizing samples for the program to
	/// subsequently read.
        virtual void
		start();
    
        /// Stop the audio device.
        /// Input devices: stop digitizing samples.
        virtual void
		stop();
  };

  AudioInALSA::AudioInALSA(const char * p)
  : AudioInput("alsa", p), handle(0), parameters(strdup(p)), started(false)
  {
    handle = ALSASetup(
     p,
     SND_PCM_STREAM_CAPTURE,
     0,
     SND_PCM_FORMAT_S16,
     SND_PCM_ACCESS_RW_INTERLEAVED,
     1,
     SampleRate,
     AudioFrameSamples, 
     AudioFrameSamples * 2);

    if ( handle == 0 )
      do_throw(-ENODEV);

    snd_pcm_pause(handle, 1);
  }

  AudioInALSA::~AudioInALSA()
  {
    snd_pcm_drop(handle);
    snd_pcm_close(handle);
    free(parameters);
  }

  NORETURN void
  AudioInALSA::do_throw(const int error, const char * message)
  {
    std::ostringstream str;

    str << "Error on ALSA audio input \"" << parameters << "\": ";
     if ( message )
       str << message << ": ";
     str << snd_strerror(error) << '.';
    throw std::runtime_error(str.str().c_str());
  }

  // Read audio into the "short" type.
  std::size_t
  AudioInALSA::read16(std::int16_t * array, std::size_t length)
  {
    int result = snd_pcm_readi(handle, array, length);
    started = true;
    if ( result == -EPIPE ) {
      snd_pcm_recover(handle, result, 1);
      snd_pcm_start(handle);
      std::cerr << "ALSA input \"" << parameters << "\": read underrun." << std::endl;
      return 0;
    }
    if ( result >= 0 ) {
      return result;
    }
    else
      do_throw(result, "Read");
  }

  AudioInput *
  Driver::AudioInALSA(const char * parameter)
  {
    return new ::FreeDV::AudioInALSA(parameter);
  }

  static std::ostream &
  AudioInALSAEnumerator(std::ostream & stream)
  {
    return ALSAEnumerate(stream, SND_PCM_STREAM_CAPTURE);
  }

  int
  AudioInALSA::poll_fds(PollType * array, int space)
  {
    const int size = snd_pcm_poll_descriptors_count(handle);
    
    snd_pcm_poll_descriptors(
     handle,
     array,
     space);
    
    return size;
  }

  std::size_t
  AudioInALSA::ready()
  {
    snd_pcm_sframes_t	available = 0;
    snd_pcm_sframes_t	delay = 0;
    int			error;

    if ( !started ) {
      start();
      return AudioFrameSamples;
    }

    error = snd_pcm_avail_delay(handle, &available, &delay);

    // If the program has been paused, there will be a long queue of incoming
    // audio samples and a corresponding delay. This can also happen if the
    // incoming audio interface runs at a faster rate than the outgoing one.
    if ( delay >= overlong_delay && available > 0 ) {
      snd_pcm_drop(handle);
      snd_pcm_prepare(handle);
      snd_pcm_start(handle);

      const double seconds = (double)delay / (double)SampleRate;

      error = snd_pcm_avail_delay(handle, &available, &delay);

      std::cerr << "ALSA input \"" << parameters << "\": overlong delay, dropped "
       << seconds << " seconds of input." << std::endl;

      return 1;
    }

    if ( error == -EPIPE ) {
      snd_pcm_recover(handle, error, 1);
      snd_pcm_start(handle);
      std::cerr << "ALSA input \"" << parameters << "\": ready underrun." << std::endl;
      return 0;
    }

    if ( error >= 0 )
      return available;
    else
      do_throw(error, "Get Frames Available for Read");
  }

  void
  AudioInALSA::start()
  {
    snd_pcm_drop(handle);
    snd_pcm_prepare(handle);
    snd_pcm_start(handle);
    started = true;
  }

  void
  AudioInALSA::stop()
  {
    snd_pcm_drop(handle);
    snd_pcm_prepare(handle);
    snd_pcm_pause(handle, 1);
    started = false;
  }

  static bool
  initializer()
  {
    driver_manager()->register_audio_input(
     "alsa",
     Driver::AudioInALSA,
     AudioInALSAEnumerator);
    return true;
  }
  static const bool initialized = initializer();
}
