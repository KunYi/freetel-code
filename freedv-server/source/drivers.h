/// FreeDV driver interface definitions.
#include <cstdint>
#include <iostream>


/// Namespace used for all code in this program.
namespace FreeDV {

  /// Allocate memory and copy a string into it, so that it is permanently
  /// stored.
  /// \param s The string to be copied.
  /// \return The new copy. It's the caller's responsibility to free this data,
  ///  or a memory leak will occurr.
  char * copy_string(const char * s);

  class Base {
  private:
	/// The copy constructor is private to prevent it from being used.
	/// \param that Not used.
			Base(const Base & that);
	
	/// The assignment operator is private to prevent it from being used.
	/// \param that Not used.
	Base &		operator = (const Base & that);

  protected:
	/// The name of the driver. This must be the same as the name it
	/// is registered under. It's expected to be from a per-class static
	/// string and thus should not be deleted.
	const char * const
			name;

	/// The parameters to this instance of the driver. They are
	/// copied here so that we can print them later in operator<<() .
	/// the copy is deleted when in the ~Base() destructor.
	const char * const
			parameters;

	/// Constructor for the virtual base class.
	/// \param _name Name of the driver. This is expected to be a single
	///  constant static string per driver class.
	/// \param _parameters Driver-specific configuration parameters.
			Base(const char * _name, const char * _parameters);

	/// Destroy the base class.
	virtual		~Base();

  public:
	/// Return true if the object is owned by a UserInterface object and
	/// should not be destroyed separately.
	/// The return value is invariant for a particular object (or possibly
	/// class).
	virtual bool const
			captive() const;

	/// Write the driver information onto a stream, for debugging and
	/// for dumping the configuration information.
	/// \param stream A reference to an instance of ostream upon which the
	///  object information is to be rendered.
	/// \return A reference to the provided stream, meant for the
	///  usual successive call paradigm of ostream operator << .
	std::ostream &	print(std::ostream &) const;
  };

  /// Write the driver information from the Base object onto a stream,
  /// for debugging and dumping the configuration information.
  /// \param stream A reference to an instance of ostream upon which the
  ///  object information is to be rendered.
  /// \return A reference to the provided stream, meant for the
  ///  usual successive call paradigm of ostream operator << .
  inline std::ostream &
  operator << (std::ostream & stream, const Base & base) {
    return base.print(stream);
  }
  
  /// Virtual base class for audio input drivers.
  class AudioInput : public ::FreeDV::Base {
  protected:
	/// Create an AudioInput device instance.
	/// \param parameters Driver-specific configuration parameters.
  			AudioInput(const char * name, const char * parameters);

	/// Destroy an AudioInput device instance.
	virtual		~AudioInput() = 0;

  public:
	/// Get the current audio level.
	/// \return The current audio level.
	/// The value is normalized to the range of 0.0 to 1.0.
	virtual float	level() = 0;

	/// Set the current audio level.
	/// \param value The new value for the current audio level.
	/// The value must be normalized within the range of 0.0 to 1.0.
	virtual void	level(float value) = 0;

        /// Read audio into an array of the signed 16-bit integer type.
	virtual std::size_t
			read16(int16_t * array, std::size_t length) = 0;
  };

  /// Virtual base class for audio output drivers.
  class AudioOutput : public ::FreeDV::Base {
  protected:
	/// Create an AudioOutput device instance.
	/// \param parameters Driver-specific configuration parameters.
  			AudioOutput(const char * name, const char * parameters);

	/// Destroy an AudioOutput device instance.
	virtual		~AudioOutput() = 0;

  public:
	/// Get the current audio level.
	/// The value is normalized to the range of 0.0 to 1.0.
	virtual float	level() = 0;

	/// Set the current audio level.
	/// The value must be within the range of 0.0 to 1.0.
	virtual void	level(float value) = 0;

        /// Write audio from an array of the signed 16-bit integer type.
	virtual std::size_t
			write16(const int16_t * array, std::size_t length) = 0;
  };

  /// Virtual base class for codecs.
  class Codec : public ::FreeDV::Base {
  protected:
	/// Create a codec instance.
	/// \param name Name of the driver. This is expected to be a single
	///  constant static string per driver class.
	/// \param parameters Driver-specific configuration parameters.
  			Codec(const char * name, const char * parameters);

	/// Destroy a codec instance.
	virtual		~Codec() = 0;

  public:
	/// Encode from an array of the signed 16-bit integer type to an
	/// array of the unsigned 8-bit integer type (this is usually
	/// implemented as unsigned char).
	/// \param i The array of audio samples to be encoded, in an array
	/// of signed 16-bit integers.
	/// \param o The encoded data, in an array of unsigned 8-bit integers.
	/// \param length The number of audio samples to be encoded.
	/// \return The number of uint8_t elements in the encoded array.
	virtual std::size_t
			encode16(const int16_t * i, uint8_t * o, \
         		 std::size_t length) = 0;

	/// Return the size of uint8_t array necessary to encode the given
	/// number of audio samples. Sample rate is 8K samples/second.
	/// The result is invariant for a given input.
	/// \param sample_count The number of audio samples to encode.
	/// Must be a multiple of frame_size().
	/// \return The size of uint8_t array necessary to encode the given
	/// number of audio samples.
	virtual std::size_t const	
			encoded_buffer_size(std::size_t sample_count) const = 0;

	/// Return the duration of a frame in milliseconds.
	/// \return The duration of a frame in milliseconds.
	virtual int const
			frame_duration() const = 0;

	/// Return the number of audio samples necessary to decode the given
	/// encoded buffer size. Sample rate is 8K samples/second.
	/// \param buffer_size is the size of the encoded buffer. It must
	///  encode a multiple of frame_size() audio samples.
	/// \return The number of audio samples necessary to decode the given
	/// encoded buffer size. The result is invariant for a given input.
	virtual std::size_t const	
			samples_per_buffer(std::size_t buffer_size) const = 0;

	/// Return the number of audio samples expected to create a codec
	/// frame. Samples provided to encode_buffer_size must be a multiple
	/// of this value. Sample rate is 8K samples/second.
	/// The result for a particular input is invariant.
	/// \return The number of audio samples expected to create a codec
	/// frame.
	virtual std::size_t const
			samples_per_frame() const = 0;
  };

  /// Event handler class, indirects the event handler of the particular GUI
  /// software or POSIX.
  class EventHandler : public ::FreeDV::Base {
  private:
	bool		do_exit;

  protected:
	/// Bit field of status values for file descriptor events.
	/// This is an argument to the coroutine called by monitor().
	/// This is a simplification on all of the values that POSIX
	/// poll() can return. Events that aren't read or write are mapped
	/// to one of those.

	/// File being monitored is readable or has read error.
	const unsigned int Read = 1;

	/// File being monitored is writable or has write error.
	const unsigned int Write = 2;

	/// Create an event handler instance.
			EventHandler(const char * name, const char * parameters)
			: Base(name, parameters), do_exit(false)
			{
			}
	virtual		~EventHandler() = 0;

	/// If set_exit() has been called, return true once.
	/// \return True if set_exit has been called. The next and subsequent
	///  calls will return false until set_exit() is called again.
	inline bool	get_exit() {
			  if ( do_exit ) {
			    do_exit = false;
			    return true;
			  }
			  else
			    return false;
			}

	/// Run one iteration of the event handler.
	/// The default implementation throws std::runtime_error.
	///
	/// If iterate() is not implemented in the child class,
	/// the child class must implement loop(), and the child class
	/// implementation of loop() must not call iterate().
	void		iterate();

	/// Cause get_exit() to return true the next time it is called.
	inline void	set_exit() { do_exit = true; }
  public:
	/// Run the event loop.
	/// The default implementation iterates checking get_exit(), returning
	/// if its value is true and otherwise and calling iterate().
	/// If you provide your own implementation of loop(), you must check
	/// get_exit() and return from this method if its value is true.
	/// If you provide your own implementation of loop(), it's your choice
	/// whether or not to implement and call iterate().
	void		loop();

	/// Monitor a file descriptor in the event loop. Call a function if the
	/// file descriptor is ready for I/O.
	/// \param fd The file descriptor to monitor.
	/// \param type A bit-field of values defined in this class,
	///  indicating the kinds of events to listen for.
	/// \param private_data Private data to be passed to the event
	///  function.
	/// \param event A coroutine to call when there is a status change
	///  on the file descriptor. The arguments of the coroutine are
	///  - fd: The file descriptor that has an event.
	///  - type: A bit-field of FDStatus values indicating the events
	///    received.
	///  - private: The address of opaque data to be passed to the driver.
	virtual void	monitor(int fd, unsigned int type, void * private_data,
			 void (*event)(int fd, unsigned int type, void * private_data)
			 ) = 0;

	/// Remove all monitoring of the given file descriptor by the event
	/// loop handler.
	/// \param fd The file descriptor to be removed from monitoring.
	virtual void	unmonitor(int fd) = 0;
  };

  /// Radio device keying driver.
  class Keying : public ::FreeDV::Base {
  protected:
	/// Create an radio keying output device instance.
	/// \param name Name of the driver. This is expected to be a single
	///  constant static string per driver class.
	/// \param parameters Driver-specific configuration parameters.
  			Keying(const char * name, const char * parameters);

	/// Destroy the radio keying device instance.
	virtual		~Keying() = 0;

  public:
	/// Key or un-key the transmitter.
	/// \param value If true, key the transmitter. If false, un-key.
	virtual void	key(bool value) = 0;
  };

  /// Softmodem driver.
  class Modem : public ::FreeDV::Base {
  protected:
	/// Create a softmodem device instance.
	/// \param name Name of the driver. This is expected to be a single
	///  constant static string per driver class.
	/// \param parameters Driver-specific configuration parameters.
  			Modem(const char * name, const char * parameters);

	virtual		~Modem() = 0;
  };

  /// Push-to-talk input driver.
  class PTTInput : public ::FreeDV::Base {
  private:
	/// Coroutine to be called when the sense of the push-to-talk switch
	/// changes.
	void	(*callback)(bool);

  protected:
	/// The driver calls this member to inform FreeDV that the PTT switch
	/// sense has changed. The value is true for key-down,
	/// false for key-up.
	/// \param value True for key-down, false for key-up.
	void		changed(bool value);

	/// Create a push-to-talk switch instance.
	/// \param name Name of the driver. This is expected to be a single
	///  constant static string per driver class.
	/// \param parameters Driver-specific configuration parameters.
  			PTTInput(const char * name, const char * parameters);

	virtual		~PTTInput() = 0;

  public:
	/// Set the function that will be called when the push-to-talk input
	/// changes its value.
	void		set_callback(void (*value)(bool));
  };

  /// Driver for the text message source function.
  class TextInput : public ::FreeDV::Base {
  protected:
	/// The child class calls this member in its parent to set the text.
	void		set(const char * text);


	/// Create a text source instance.
	/// \param name Name of the driver. This is expected to be a single
	///  constant static string per driver class.
	/// \param parameters Driver-specific configuration parameters.
  			TextInput(const char * name, const char * parameters);

	virtual		~TextInput() = 0;
  };

  class Interfaces;

  /// Generic base class for user interfaces.
  /// They may be graphical, they may be server-client interfaces,
  /// they may be specialized hardware devices, especially if this
  /// software is embedded.
  /// There must be inputs and callbacks for many things here.
  /// UserInterfaces may provide their own drivers for microphone,
  /// loudspeaker, TextInput, both forms of PTT, and EventHandler.
  class UserInterface : public ::FreeDV::Base {
  protected:
	/// The external Interfaces object.
	Interfaces *
		interfaces;

	/// Create an instance of the UserInterface object.
	/// \param name Name of the driver. This is expected to be a single
	///  constant static string per driver class.
	/// \param parameters Driver-specific configuration parameters.
	/// \param interfaces An Interface object. The UserInterface object
	/// may set various fields of Interface to be its own captive driver
	/// objects, and they may change during operation if the user changes
	/// device driver parameters.
  				UserInterface(const char * name, const char * parameters, Interfaces * _interfaces);

	virtual			~UserInterface() = 0;
  };

  /// Structure used to pass all of the drivers. Can be modified from
  /// UserInterface while within the event loop, if a user changes a device
  /// from the GUI.
  class Interfaces {
    public:
   			Interfaces() : codec(0), event_handler(0),
			 keying_output(0), loudspeaker(0), microphone(0),
			 modem(0), ptt_input_digital(0), ptt_input_ssb(0),
                         receiver(0), text_input(0), transmitter(0),
			 user_interface(0)
			{
			}

    /// The voice codec in use.
    Codec *		codec;
    /// The event loop handler. This is specific to a GUI, or POSIX.
    EventHandler *	event_handler;
    /// The output used to key the transmitter.
    Keying *		keying_output;
    /// The audio output which drives the loudspeaker or headphone.
    AudioOutput *	loudspeaker;
    /// The audio input from the microphone.
    AudioInput *	microphone;
    /// The softmodem.
    Modem *		modem;
    /// The PTT input that indicates the transmission is to be digital audio.
    PTTInput *		ptt_input_digital;
    /// The PTT input that indicates the transmission is to be SSB.
    PTTInput *		ptt_input_ssb;
    /// The text to be transmitted in our text side-channel.
    TextInput *		text_input;
    /// The audio output that drives the transmitter.
    AudioOutput *	transmitter;
    /// The audio input from the receiver.
    AudioInput *	receiver;
    /// The user interface driver. Used for GUIs.
    UserInterface *	user_interface;

    /// Fill in default drivers if the user or UserInterface hasn't set any.
    void		fill_in();

    /// Write the command-line flags necessary to configure the drivers as 
    /// they are presently configured to the stream. This is used to save
    /// the configuration or debug the program.
    /// \param stream A reference to an instance of ostream on which the
    /// \return A reference to the provided stream, meant for the
    ///  usual successive call paradigm of ostream operator << .
    virtual std::ostream &
			print(std::ostream &) const;
  };
  /// Write the driver information from the Interfaces object onto a stream,
  /// for debugging and dumping the configuration information.
  /// \param stream A reference to an instance of ostream upon which the
  ///  object information is to be rendered.
  /// \return A reference to the provided stream, meant for the
  ///  usual successive call paradigm of ostream operator << .
  inline std::ostream &
  operator << (std::ostream & stream, const Interfaces & interfaces) {
    return interfaces.print(stream);
  }

  namespace Driver {
    AudioInput *	Tone(const char * parameter);
    AudioOutput *	AudioSink(const char * parameter);
    Codec *		CodecNoOp(const char * parameter);
    Keying *		KeyingSink(const char * parameter);
    Modem *		ModemNoOp(const char * parameter);
    PTTInput *		PTTConstant(const char * parameter);
    TextInput *		TextConstant(const char * parameter);
    UserInterface *	BlankPanel(const char * parameter, Interfaces *);
  };
}

#ifndef NO_INITIALIZERS
#include <map>
#include <string>

/// Namespace used for the entire program.
namespace FreeDV {
  /// Device driver manager. Allows for registration and enumeration of device
  /// drivers. Instantiates device drivers on request.
  class DriverManager {
  private:
			std::map<std::string, AudioInput *(*)(const char *)> audio_input_drivers;
			std::map<std::string, AudioOutput *(*)(const char *)> audio_output_drivers;
			std::map<std::string, Codec *(*)(const char *)> codecs;
			std::map<std::string, Keying *(*)(const char *)> keying_output_drivers;
			std::map<std::string, Modem *(*)(const char *)> modems;
			std::map<std::string, PTTInput *(*)(const char *)> ptt_input_drivers;
			std::map<std::string, TextInput *(*)(const char *)> text_input_drivers;
			std::map<std::string, UserInterface *(*)(const char *, Interfaces *)> user_interface_drivers;
  public:

	/// Initialize the driver manager.
			DriverManager();
			~DriverManager();

	/// Print the available drivers to the argument stream.
	/// \param stream A reference to an instance of ostream on which the
	///  information is to be printed.
        std::ostream &	print(std::ostream & stream) const;

	/// Instantiate an AudioInput driver.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	AudioInput *	audio_input(const char * driver, const char * parameters);
	/// Instantiate an AudioOutput driver.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	AudioOutput *	audio_output(const char * driver, const char * parameters);
	/// Instantiate a Codec.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	Codec *		codec(const char * driver, const char * parameters);
	/// Instantiate a Keying driver.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	Keying *	keying_output(const char * driver, const char * parameters);
	/// Instantiate a softmodem.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	Modem *		modem(const char * driver, const char * parameters);
	/// Instantiate a PTT input driver.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	PTTInput *	ptt_input(const char * driver, const char * parameters);
	/// Instantiate a text input driver.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	TextInput *	text_input(const char * driver, const char * parameters);
	/// Instantiate a user interface driver.
	/// \param driver The name of the driver.
	/// \param parameters Driver-specific configuration parameters.
	/// \param interfaces Interfaces object used to hold all of the
	///  current device driver instances.
	UserInterface *	user_interface(const char * driver, const char * parameters, Interfaces * interfaces);

	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_audio_input(const char * driver, AudioInput * (*creator)(const char *));
	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_audio_output(const char * driver, AudioOutput * (*creator)(const char *));
	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_codec(const char * driver, Codec * (*creator)(const char *));
	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_keying_output(const char * driver, Keying * (*creator)(const char *));
	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_modem(const char * driver, Modem * (*creator)(const char *));
	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_ptt_input(const char * driver, PTTInput * (*creator)(const char *));
	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_text_input(const char * driver, TextInput * (*creator)(const char *));
	/// Register an audio input driver.
	/// \param driver The name of the driver.
	/// \param creator The coroutine that will instantiate the driver.
	void		register_user_interface(const char * driver, UserInterface * (*creator)(const char *, Interfaces *));
  };
  /// Write the driver information from the DriverManager object onto a stream,
  /// for debugging and dumping the configuration information.
  /// \param stream A reference to an instance of ostream upon which the
  ///  object information is to be rendered.
  /// \return A reference to the provided stream, meant for the
  ///  usual successive call paradigm of ostream operator << .
  inline std::ostream &
  operator << (std::ostream & stream, const DriverManager & d) {
    return d.print(stream);
  }
  

  /// Global reference to the driver manager.
  extern DriverManager & driver_manager;

  /// Return a reference to the driver manager instance.
  /// This is a function because it is called in static initializers.
  extern DriverManager & init_driver_manager();
#endif
}
