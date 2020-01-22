#pragma once
#include <r2/utilities/buffer.h>
#include <r2/utilities/dynamic_array.hpp>

namespace r2 {
	enum texture_type {
		tt_byte = 0,
		tt_unsigned_byte,
		tt_short,
		tt_unsigned_short,
		tt_int,
		tt_unsigned_int,
		tt_float
	};

	enum texture_min_filter {
		tmnf_nearest = 0,
		tmnf_linear,
		tmnf_nearest_mipmap_nearest,
		tmnf_linear_mipmap_nearest,
		tmnf_nearest_mipmap_linear,
		tmnf_linear_mipmap_linear
	};

	enum texture_mag_filter {
		tmgf_nearest = 0,
		tmgf_linear
	};

	enum texture_wrap {
		tw_clamp_to_edge = 0,
		tw_mirrored_repeat,
		tw_repeat
	};

	class texture_buffer : public gpu_buffer {
		public:
			virtual void* data() const { return m_data; }

			void create(u8* data, u32 width, u32 height, u8 channels, texture_type type);
			void create(u32 width, u32 height, u8 channels, texture_type type);

			void set_pixel(u32 x, u32 y, u8 channel, void* data);
			void set_pixel(u32 x, u32 y, void* data);

			void set_min_filter(texture_min_filter filter);
			void set_mag_filter(texture_mag_filter filter);
			void set_x_wrap_mode(texture_wrap x);
			void set_y_wrap_mode(texture_wrap y);
			inline texture_min_filter min_filter() const { return m_minFilter; }
			inline texture_mag_filter mag_filter() const { return m_magFilter; }
			inline texture_wrap wrap_x() const { return m_wrapX; }
			inline texture_wrap wrap_y() const { return m_wrapY; }
			inline bool has_mode_updates() { return m_minFilterChanged || m_magFilterChanged || m_wrapXChanged || m_wrapYChanged; }
			inline bool min_filter_updated() const { return m_minFilterChanged; }
			inline bool mag_filter_updated() const { return m_magFilterChanged; }
			inline bool wrap_x_updated() const { return m_wrapXChanged; }
			inline bool wrap_y_updated() const { return m_wrapYChanged; }
			void clear_mode_updates();

			template <typename P>
			P pixel(u32 x, u32 y) {
				return *((P*)m_data)[x + (y * m_width)];
			}

			template <typename P>
			void pixel(u32 x, u32 y, const P& data) {
				set_pixel(x, y, &data);
			}

			inline u32 width() const { return m_width; }
			inline u32 height() const { return m_height; }
			inline u8 channels() const { return m_channelCount; }
			inline u8 bytes_per_channel() const { return m_bytesPerChannel; }
			inline texture_type type() const { return m_type; }

		protected:
			friend class scene;
			texture_buffer();
			~texture_buffer();

			u8* m_data;
			u32 m_width;
			u32 m_height;
			u8 m_channelCount;
			u8 m_bytesPerChannel;
			texture_type m_type;
			texture_min_filter m_minFilter;
			texture_mag_filter m_magFilter;
			texture_wrap m_wrapX;
			texture_wrap m_wrapY;
			bool m_minFilterChanged;
			bool m_magFilterChanged;
			bool m_wrapXChanged;
			bool m_wrapYChanged;
	};

	enum render_buffer_depth_mode {
		rbdm_16_bit = 0,
		rbdm_24_bit,
		rbdm_32_bit,
		rbdm_float,
		rbdm_no_depth
	};

	class render_buffer : public gpu_buffer {
		public:
			virtual void* data() const { return nullptr; }
			inline size_t attachment_count() const { return m_attachments.size(); }
			inline texture_buffer* attachment(size_t idx) { return *m_attachments[idx]; }
			inline render_buffer_depth_mode depth_mode() const { return m_depth; }
			inline bool depth_mode_changed() const { return m_depthModeChanged; }
			inline bool has_mode_updates() const { return m_depthModeChanged; }
			vec2i dimensions();
			void clear_mode_updates();
			void raise_synced_flag();

			void attach(texture_buffer* texture);
			void set_depth_mode(render_buffer_depth_mode mode);

			void fetch_pixel(u32 x, u32 y, size_t attachmentIdx, void* dest, size_t pixelSize);

			template <typename P>
			P pixel(u32 x, u32 y, size_t attachmentIdx) {
				P pixel;
				fetch_pixel(x, y, attachmentIdx, &pixel, sizeof(P));
				return pixel;
			}

		protected:
			friend class scene;
			render_buffer();
			~render_buffer();

			bool m_wasSynced;
			bool m_depthModeChanged;
			render_buffer_depth_mode m_depth;
			dynamic_pod_array<texture_buffer*> m_attachments;
	};
};
