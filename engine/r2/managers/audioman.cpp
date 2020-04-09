#include <r2/managers/audioman.h>
#include <r2/engine.h>

namespace r2 {
	bool check_al_error() {
		ALenum err = alGetError();
		if (err != AL_NO_ERROR) {
			mstring e = "";
			if (err == AL_INVALID_NAME) e = "AL_INVLAID_NAME";
			if (err == AL_INVALID_ENUM) e = "AL_INVALID_ENUM";
			if (err == AL_INVALID_VALUE) e = "AL_INVALID_VALUE";
			if (err == AL_INVALID_OPERATION) e = "AL_INVALID_OPERATION";
			if (err == AL_OUT_OF_MEMORY) e = "AL_OUT_OF_MEMORY";

			r2Error("OpenAL error: %s", e.c_str());
			return true;
		}

		return false;
	}



	audio_buffer::audio_buffer() {
		m_buffer = UINT_MAX;
		m_duration = 0.0f;
	}

	audio_buffer::~audio_buffer() {
		if (m_buffer != UINT_MAX) alDeleteBuffers(1, &m_buffer);
	}


	bool audio_buffer::fill(audio_source_format format, void* data, size_t data_size, size_t sampleRate) {
		if (m_buffer != UINT_MAX) {
			alDeleteBuffers(1, &m_buffer);
			m_buffer = UINT_MAX;
		}

		alGenBuffers(1, &m_buffer);
		if (check_al_error()) {
			r2Error("Failed to create OpenAL buffer");
			return false;
		}

		alBufferData(m_buffer, format, data, data_size, sampleRate);
		bool ret = check_al_error();
		if (!ret) {
			size_t sample_size = 1;
			size_t channels = 1;
			if (format == asf_stereo_8bit) channels = 2;
			else if (format == asf_mono_16bit) sample_size = 2;
			else if (format == asf_stereo_16bit) {
				sample_size = 2;
				channels = 2;
			}
			m_duration = f32((data_size / channels) / sample_size) / f32(sampleRate);
		} else {
			r2Error("Failed to fill OpenAL buffer with data");
			alDeleteBuffers(1, &m_buffer);
			m_buffer = UINT_MAX;
		}

		return !ret;
	}



	audio_source::audio_source(audio_buffer* buf) {
		m_source = UINT_MAX;
		alGenSources(1, &m_source);
		check_al_error();

		if (buf) {
			m_buffer = buf;
			m_allocatedOwnBuffer = false;
			alSourcei(m_source, AL_BUFFER, m_buffer->bufferId());
			
		} else {
			m_buffer = new audio_buffer();
			m_allocatedOwnBuffer = true;
		}

		setPosition(vec3f(0, 0, 0));
		setVelocity(vec3f(0, 0, 0));
		setPitch(1.0f);
		setGain(1.0f);
		setDoesLoop(false);

		m_maxDistance = 0.0f;
		m_rolloffFactor = 1.0f;
		m_referenceDistance = 0.0f;
		m_minGain = 0.0f;
		m_maxGain = 0.0f;
		m_coneOuterGain = 0.0f;
		m_coneInnerAngle = 0.0f;
		m_coneOuterAngle = 360.0f;
		m_direction = vec3f(0, 0, 0);
	}

	audio_source::~audio_source() {
		if (m_source != UINT_MAX) alDeleteSources(1, &m_source);
		if (m_allocatedOwnBuffer && m_buffer) delete m_buffer;
		check_al_error();
	}

	void audio_source::setPitch(f32 pitch) {
		alSourcef(m_source, AL_PITCH, pitch);
		if (!check_al_error()) m_pitch = pitch;
	}

	void audio_source::setGain(f32 gain) {
		alSourcef(m_source, AL_GAIN, gain);
		if (!check_al_error()) m_gain = gain;
	}

	void audio_source::setMinGain(f32 min) {
		alSourcef(m_source, AL_MIN_GAIN, min);
		if (!check_al_error()) m_minGain = min;
	}

	void audio_source::setMaxGain(f32 max) {
		alSourcef(m_source, AL_MIN_GAIN, max);
		if (!check_al_error()) m_maxGain = max;
	}

	void audio_source::setMaxDistance(f32 distance) {
		alSourcef(m_source, AL_MAX_DISTANCE, distance);
		if (!check_al_error()) m_maxDistance = distance;
	}

	void audio_source::setRolloffFactor(f32 factor) {
		alSourcef(m_source, AL_ROLLOFF_FACTOR, factor);
		if (!check_al_error()) m_rolloffFactor = factor;
	}

	void audio_source::setReferenceDistance(f32 distance) {
		alSourcef(m_source, AL_REFERENCE_DISTANCE, distance);
		if (!check_al_error()) m_referenceDistance = distance;
	}

	void audio_source::setConeOuterGain(f32 gain) {
		alSourcef(m_source, AL_CONE_OUTER_GAIN, gain);
		if (!check_al_error()) m_coneOuterGain = gain;
	}

	void audio_source::setConeInnerAngle(f32 angle) {
		alSourcef(m_source, AL_CONE_INNER_ANGLE, angle);
		if (!check_al_error()) m_coneInnerAngle = angle;
	}

	void audio_source::setConeOuterAngle(f32 angle) {
		alSourcef(m_source, AL_CONE_OUTER_ANGLE, angle);
		if (!check_al_error()) m_coneOuterAngle = angle;
	}


	void audio_source::setPlayPosition(f32 time) {
		alSourcef(m_source, AL_SEC_OFFSET, time);
		check_al_error();
	}

	void audio_source::setPosition(const vec3f& pos) {
		alSource3f(m_source, AL_POSITION, pos.x, pos.y, pos.z);
		if (!check_al_error()) m_position = pos;
	}

	void audio_source::setVelocity(const vec3f& vel) {
		alSource3f(m_source, AL_VELOCITY, vel.x, vel.y, vel.z);
		if (!check_al_error()) m_velocity = vel;
	}

	void audio_source::setDirection(const vec3f& dir) {
		alSource3f(m_source, AL_DIRECTION, dir.x, dir.y, dir.z);
		if (!check_al_error()) m_direction = dir;
	}

	void audio_source::setDoesLoop(bool loops) {
		alSourcei(m_source, AL_LOOPING, loops ? AL_TRUE : AL_FALSE);
		if (!check_al_error()) m_loops = loops;
	}

	bool audio_source::buffer(audio_source_format format, void* data, size_t data_size, size_t sampleRate) {
		if (m_source == UINT_MAX) {
			r2Error("Audio source has invalid OpenAL source");
			return false;
		}

		if (m_buffer->fill(format, data, data_size, sampleRate)) {
			alSourcei(m_source, AL_BUFFER, m_buffer->bufferId());
		}
	}

	void audio_source::buffer(audio_buffer* buffer) {
		if (m_source == UINT_MAX) {
			r2Error("Audio source has invalid OpenAL source");
			return;
		}

		if (m_buffer && m_buffer != buffer && m_allocatedOwnBuffer) {
			delete m_buffer;
			m_buffer = nullptr;
			m_allocatedOwnBuffer = false;
		}

		alSourcei(m_source, AL_BUFFER, buffer->bufferId());
		m_buffer = buffer;
	}

	void audio_source::play() {
		alSourcePlay(m_source);
		check_al_error();
	}

	void audio_source::pause() {
		alSourcePause(m_source);
		check_al_error();
	}

	void audio_source::stop() {
		alSourceStop(m_source);
		check_al_error();
	}

	f32 audio_source::playPosition() {
		f32 r = 0.0f;
		alGetSourcef(m_source, AL_SEC_OFFSET, &r);
		return r;
	}

	bool audio_source::isPlaying() {
		ALint v = AL_INITIAL;
		alGetSourcei(m_source, AL_SOURCE_STATE, &v);
		return v == AL_PLAYING;
	}

	bool audio_source::isPaused() {
		ALint v = AL_INITIAL;
		alGetSourcei(m_source, AL_SOURCE_STATE, &v);
		return v == AL_PAUSED;
	}

	bool audio_source::isStopped() {
		ALint v = AL_INITIAL;
		alGetSourcei(m_source, AL_SOURCE_STATE, &v);
		return v == AL_STOPPED;
	}



	audio_man::audio_man() : m_device(nullptr), m_context(nullptr) {
		m_device = alcOpenDevice(NULL);
		if (!m_device) {
			r2Error("Failed to open audio device");
			return;
		}

		m_context = alcCreateContext(m_device, NULL);
		if (!m_context) {
			r2Error("Failed to create audio context");
			alcCloseDevice(m_device);
			m_device = nullptr;
			return;
		}

		if (!alcMakeContextCurrent(m_context)) {
			r2Error("Failed to activate audio context");
			alcDestroyContext(m_context);
			m_context = nullptr;
			alcCloseDevice(m_device);
			m_device = nullptr;
			return;
		}

		setListener(mat4f(1.0f));

		r2Log("Audio manager initialized");
	}

	audio_man::~audio_man() {
		if (!m_device) return;
		alcDestroyContext(m_context);
		m_context = nullptr;
		alcCloseDevice(m_device);
		m_device = nullptr;
	}

	void audio_man::setListener(const mat4f& transform, const vec3f& velocity) {
		vec3f position = transform[3];
		vec3f up = transform[1];
		vec3f forward = transform[2];
		vec3f orientation[2] = { up, forward };
		
		alListener3f(AL_POSITION, position.x, position.y, position.z);
		if (check_al_error()) m_listenerPosition = position;
		
		alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
		if (check_al_error()) m_listenerVelocity = velocity;
		
		alListenerfv(AL_ORIENTATION, &orientation[0].x);
		if (check_al_error()) {
			m_listenerUp = up;
			m_listenerForward = forward;
		}
	}

	void audio_man::setListenerPosition(const vec3f& position) {
		alListener3f(AL_POSITION, position.x, position.y, position.z);
		if (check_al_error()) m_listenerPosition = position;
	}

	void audio_man::setListenerVelocity(const vec3f& velocity) {
		alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
		if (check_al_error()) m_listenerVelocity = velocity;
	}

	void audio_man::setListenerOrientation(const vec3f& up, const vec3f& forward) {
		vec3f orientation[2] = { up, forward };
		alListenerfv(AL_ORIENTATION, &orientation[0].x);
		if (check_al_error()) {
			m_listenerUp = up;
			m_listenerForward = forward;
		}
	}
};