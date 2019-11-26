#pragma once
#include <queue>
#include <glad/glad.h>
#include <mutex>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/common.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>	
	#include<libavutil/opt.h>
}
class VideoPlayer
{
public:
	bool Init();
	bool NextFrame(AVFrame ** frame);
	bool ProcessFrame(int displayW, int displayH);
	bool SubmitFrame(uint8_t* buffer, size_t len, int width, int height);
	void Close();
private:
	AVCodec* decoder = nullptr;
	AVCodecContext* context = nullptr;
	AVCodecParserContext* parser = nullptr;
	AVFrame* frame = nullptr, * frame1 = nullptr, * frame2 = nullptr, * aux = nullptr;
	AVPacket* packet = nullptr;
	void* av_buffer;
	SwsContext* swsContext = nullptr;
	bool hasFrame = false;
	std::mutex mutex;
};

