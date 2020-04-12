#pragma once
#include <r2/config.h>

#include <al.h>
#include <alc.h>

namespace r2 {
	enum audio_source_format {
		asf_mono_8bit = AL_FORMAT_MONO8,
		asf_mono_16bit = AL_FORMAT_MONO16,
		asf_stereo_8bit = AL_FORMAT_STEREO8,
		asf_stereo_16bit = AL_FORMAT_STEREO16
	};

	class audio_buffer {
		public:
			audio_buffer();
			~audio_buffer();

			inline const f32 duration() const { return m_duration; }
			inline const ALuint bufferId() const { return m_buffer; }

			bool fill(audio_source_format format, void* data, size_t data_size, size_t sampleRate);

		protected:
			ALuint m_buffer;
			f32 m_duration;
	};
	
	#define NO_AUDIO_BUFFER ((audio_buffer*)0xDEADBEEF)
	class audio_source {
		public:
			audio_source(audio_buffer* buf = nullptr);
			~audio_source();

			void setPitch(f32 pitch);
			void setGain(f32 gain);
			void setMinGain(f32 min);
			void setMaxGain(f32 max);
			void setMaxDistance(f32 distance);
			void setRolloffFactor(f32 factor);
			void setReferenceDistance(f32 distance);
			void setConeOuterGain(f32 gain);
			void setConeInnerAngle(f32 angle);
			void setConeOuterAngle(f32 angle);
			void setPlayPosition(f32 time);
			void setPosition(const vec3f& pos);
			void setVelocity(const vec3f& vel);
			void setDirection(const vec3f& dir);
			void setDoesLoop(bool loops);

			inline const f32 pitch() const { return m_pitch; }
			inline const f32 gain() const { return m_gain; }
			inline const f32 minGain() const { return m_minGain; }
			inline const f32 maxGain() const { return m_maxGain; }
			inline const f32 maxDistance() const { return m_maxDistance; }
			inline const f32 rolloffFactor() const { return m_rolloffFactor; }
			inline const f32 referenceDistance() const { return m_referenceDistance; }
			inline const f32 coneOuterGain() const { return m_coneOuterGain; }
			inline const f32 coneInnerAngle() const { return m_coneInnerAngle; }
			inline const f32 coneOuterAngle() const { return m_coneOuterAngle; }
			inline const vec3f position() const { return m_position; }
			inline const vec3f velocity() const { return m_velocity; }
			inline const vec3f direction() const { return m_direction; }
			inline const bool doesLoop() const { return m_loops; }
			inline const f32 duration() const { return m_buffer->duration(); }
			inline audio_buffer* const buffer() const { return m_buffer; }

			bool buffer(audio_source_format format, void* data, size_t data_size, size_t sampleRate);
			void buffer(audio_buffer* buffer);

			void play();
			void pause();
			void stop();

			f32 playPosition();
			bool isPlaying();
			bool isPaused();
			bool isStopped();
			
		protected:
			f32 m_pitch;
			f32 m_gain;
			f32 m_minGain;
			f32 m_maxGain;
			f32 m_maxDistance;
			f32 m_rolloffFactor;
			f32 m_referenceDistance;
			f32 m_coneOuterGain;
			f32 m_coneInnerAngle;
			f32 m_coneOuterAngle;
			vec3f m_position;
			vec3f m_velocity;
			vec3f m_direction;
			bool m_loops;

			ALuint m_source;
			audio_buffer* m_buffer;
			bool m_allocatedOwnBuffer;
	};

	class audio_man {
		public:
			audio_man();
			~audio_man();

			void setListener(const mat4f& transform, const vec3f& velocity = vec3f(0, 0, 0));
			void setListenerPosition(const vec3f& position);
			void setListenerVelocity(const vec3f& velocity);
			void setListenerOrientation(const vec3f& up, const vec3f& forward);

			inline const vec3f listenerPosition() const { return m_listenerPosition; }
			inline const vec3f listenerVelocity() const { return m_listenerVelocity; }
			inline const vec3f listenerUp() const { return m_listenerUp; }
			inline const vec3f listenerForward() const { return m_listenerForward; }

		protected:
			vec3f m_listenerPosition;
			vec3f m_listenerVelocity;
			vec3f m_listenerUp;
			vec3f m_listenerForward;

			ALCdevice* m_device;
			ALCcontext* m_context;
	};
};