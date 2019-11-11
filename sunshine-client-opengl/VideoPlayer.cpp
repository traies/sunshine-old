#include "stdafx.h"
#include "VideoPlayer.h"
#include "../easyloggingpp/easylogging++.h"

bool VideoPlayer::Init()
{
	decoder = avcodec_find_decoder(AV_CODEC_ID_HEVC);
	if (decoder == nullptr) {
		LOG(ERROR) << "No H264 decoder found.";
		return false;
	}

	context = avcodec_alloc_context3(decoder);
	context->thread_count = 1;
	parser = av_parser_init(decoder->id);
	if (!parser) {
		LOG(ERROR) << "Parser init failed";
		return false;
	}

	int res = avcodec_open2(context, decoder, nullptr);
	if (res < 0) {
		LOG(ERROR) << "avcodec open failed";
		return false;
	}

	frame = av_frame_alloc();
	if (frame == nullptr) {
		LOG(ERROR) << "Frame alloc failed";
		return false;
	}
	packet = av_packet_alloc();

	return true;
}

void VideoPlayer::Close()
{
	av_parser_close(parser);
	avcodec_free_context(&context);
	if (swsContext != nullptr) {
		sws_freeContext(swsContext);
	}
	if (av_buffer != nullptr) {
		av_free(av_buffer);
	}
	if (rgbFrame != nullptr) {
		av_frame_free(&rgbFrame);
	}
}

bool VideoPlayer::SubmitFrame(uint8_t* buffer, size_t len, int displayW, int displayH)
{
	int parsed;
	while (len > 0) {
		packet->size = 0;
		parsed = av_parser_parse2(parser, context, &packet->data, &packet->size, buffer, len, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (parsed < 0) {
			LOG(ERROR) << "Parse failed";
			return false;
		}
		buffer += parsed;
		len -= parsed;
		if (packet->size > 0) {
			int ret = avcodec_send_packet(context, packet);
			if (ret < 0) {
				LOG(ERROR) << "Send packet failed:" << ret;
				//avcodec_flush_buffers(context);
				return false;
			}

			ProcessFrame(displayW, displayH);
		}	
	}
	return true;
}

bool VideoPlayer::ProcessFrame(int displayW, int displayH)
{
	int ret = 1;
	AVFrame* aux = av_frame_alloc();
	while (ret >= 0) {
		ret = avcodec_receive_frame(context, aux);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF)) {
			return hasFrame;
		}
		else if (ret < 0) {
			LOG(ERROR) << "Recieve frame error: " < ret;
			return false;
		}

		if (hasFrame) {
			av_frame_unref(frame);
		}

		av_frame_ref(frame, aux);
		hasFrame = true;
	}

	av_frame_free(&aux);
}



bool VideoPlayer::NextFrame(AVFrame** f)
{
	if (hasFrame) {
		*f = frame;
		hasFrame = false;
		return true;
	}
	return false;
}

