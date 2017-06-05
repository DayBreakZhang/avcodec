#include "avdecoder.h"
#include "ffdecoder.h"
#include "ffhelper.h"
#include <string.h>
#include <assert.h>

//void* avdecoder_create(int codecId)
//{
//	AVCodecParameters param;
//	memset(&param, 0, sizeof(AVCodecParameters));
//	param.codec_id = codecId;
//	return ffdecoder_create(&param);
//}

void* avdecoder_create_h264()
{
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_VIDEO;
	param.codec_id = AV_CODEC_ID_H264;
	return ffdecoder_create(&param);
}

void* avdecoder_create_aac()
{
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_AAC;
	return ffdecoder_create(&param);
}

void avdecoder_destroy(void* ff)
{
	ffdecoder_destroy(ff);
}

/// @return 0-ok, other-error
int avdecoder_input(void* ff, const struct avpacket_t* pkt)
{
	AVPacket pkt2;
	if (pkt && pkt->data)
		avpacket_to_ffmpeg(pkt, 0, &pkt2);
	return ffdecoder_input(ff, pkt ? &pkt2 : NULL);
}

/// @param[in] frame must be memset to 0 or from av_frame_alloc()
/// @return >=0-got frame, <0-error
int avdecoder_getframe(void* ff, struct avframe_t* frame)
{
	AVFrame ffmpeg;
	struct avframe_t* frame2;
	memset(&ffmpeg, 0, sizeof(ffmpeg));
	int r = ffdecoder_getframe(ff, &ffmpeg);
	if (r >= 0)
	{
		frame2 = ffmpeg_to_avframe(&ffmpeg);
		if (NULL == frame2)
			return -ENOMEM;
		*frame = *frame2;
	}	
	return r;
}

int avdecoder_freeframe(void* ff, struct avframe_t* frame)
{
	return avframe_release(frame);
}

static void* aac_create(int format, int channels, int frequency)
{
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_AAC;
	return ffdecoder_create(&param);
}

static void* mp3_create(int format, int channels, int frequency)
{
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_MP3;
	return ffdecoder_create(&param);
}

struct audio_decoder_t* aac_decoder()
{
	static struct audio_decoder_t s_decoder = {
		aac_create,
		avdecoder_destroy,
		avdecoder_input,
		avdecoder_getframe,
		avdecoder_freeframe,
	};
	return &s_decoder;
}

struct audio_decoder_t* mp3_decoder()
{
	static struct audio_decoder_t s_decoder = {
		mp3_create,
		avdecoder_destroy,
		avdecoder_input,
		avdecoder_getframe,
		avdecoder_freeframe,
	};
	return &s_decoder;
}
