#pragma once
namespace Encode {
	template <typename T, typename K>
	class Encoder
	{
	public:
		Encoder() {};
		virtual ~Encoder() {};
		virtual bool PutFrame(T * frame) = 0;
		virtual bool PullBuffer(K ** buffer) = 0;
	};
}