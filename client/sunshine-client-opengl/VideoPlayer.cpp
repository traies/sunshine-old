#include "stdafx.h"
#include "VideoPlayer.h"
#include "../easyloggingpp/easylogging++.h"
#include <chrono>

//AVPixelFormat choose_nv12(AVCodecContext * context, const AVPixelFormat * format)
//{
//	while (*format != AVPixelFormat::AV_PIX_FMT_NONE) {
//		if (*format == AVPixelFormat::AV_PIX_FMT_CUDA) {
//			return AVPixelFormat::AV_PIX_FMT_CUDA;
//		}
//		format++;
//	}
//
//	LOG(ERROR) << "NV12 pixel format unsupported";
//	return AVPixelFormat::AV_PIX_FMT_NONE;
//}

bool VideoPlayer::Init()
{
	decoder = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (decoder == nullptr) {
		LOG(ERROR) << "No H264 decoder found.";
		return false;
	}

	context = avcodec_alloc_context3(decoder);
	context->thread_count = 1;
	//context->get_format = choose_nv12;
	AVBufferRef* hwDeviceCtx = nullptr;

	int res = av_hwdevice_ctx_create(&hwDeviceCtx, AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA, nullptr, nullptr, 0);
	if (res < 0) {
		LOG(ERROR) << "av_hwdevice_ctx_create failed.";
		return false;
	}
	context->hw_device_ctx = hwDeviceCtx;

	parser = av_parser_init(decoder->id);
	if (!parser) {
		LOG(ERROR) << "Parser init failed";
		return false;
	}

	res = avcodec_open2(context, decoder, nullptr);
	if (res < 0) {
		LOG(ERROR) << "avcodec open failed";
		return false;
	}

	frame1 = av_frame_alloc();
	if (frame1 == nullptr) {
		LOG(ERROR) << "Frame alloc failed";
		return false;
	}

	frame2 = av_frame_alloc();
	if (frame2 == nullptr) {
		LOG(ERROR) << "Frame alloc failed";
		return false;
	}

	frame = frame1;
	packet = av_packet_alloc();

	aux = av_frame_alloc();
	if (aux == nullptr) {
		LOG(ERROR) << "Frame alloc failed";
		return false;
	}

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
	if (aux != nullptr) {
		av_frame_free(&aux);
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
	while (ret >= 0) {
		ret = avcodec_receive_frame(context, aux);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF)) {
			return hasFrame;
		}
		else if (ret < 0) {
			LOG(ERROR) << "Recieve frame error: " < ret;
			return false;
		}

		//mutex.lock();
		av_hwframe_transfer_data(frame, aux, 0);

		hasFrame = true;

		//mutex.unlock();
	}
}



bool VideoPlayer::NextFrame(AVFrame** f)
{
	//std::lock_guard<std::mutex> lock(mutex);10
	if (hasFrame) {
		*f = frame;
		frame = frame == frame1 ? frame2 : frame1;
		hasFrame = false;
		return true;
	}
	return false;
}

