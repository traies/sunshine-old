#include "stdafx.h"
#include "D11Hook.h"
#include "AmdEncoder.h"

template<typename EncType>
PRESENT_SWAP_FUNC D11Hook<EncType>::GetPresent()
{
	if (presentSwap == nullptr) {
		// Should probably throw an exception.
		LOG(ERROR) << "Illegal call: Present ptr is null";
		ExitProcess(1);
	}
	return presentSwap;
}

template<typename EncType>
std::shared_ptr<Encode::D11EncodePipeline<EncType>> D11Hook<EncType>::GetEncodePipeline(ID3D11Device * device)
{
	if (encodePipeline == nullptr) {
		encodePipeline = std::make_shared<Encode::D11EncodePipeline<EncType>>(device, this->pipe, this->socket);
	}
	return encodePipeline;
}

template<typename EncType>
bool D11Hook<EncType>::Uninstall()
{
	return false;
}

template <typename EncType>
std::shared_ptr<D11Hook<EncType>> D11Hook<EncType>::instance;

template <typename EncType>
std::shared_ptr<D11Hook<EncType>> D11Hook<EncType>::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<D11Hook<EncType>>();
	}
	return instance;
};


template class D11Hook<Encode::AmdEncoder>;