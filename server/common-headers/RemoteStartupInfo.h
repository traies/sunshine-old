#pragma once

enum class CODEC {
	HEVC,
	H264,
};

struct RemoteProcessStartInfo {
	char	encoderAPI[16];
	char	videoIP[16];
	int		videoPort;
	uint32_t bitrate;
	CODEC	codec;
	bool	noControl;
};
