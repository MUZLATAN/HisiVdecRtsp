//
// Created by z on 2020/8/12
//

#pragma once


extern "C" {
#include <libavformat/avformat.h>
};

#include <memory>
#include <opencv2/core/core.hpp>

#include "RtspImpl.h"
#include "Vdec.h"



class RtspHisiMppImpl : public RtspImpl {
 public:
	RtspHisiMppImpl(const std::string &rtsp_addr)
			: RtspImpl(rtsp_addr),
			    inited_(false),
				pFormatCtx_(NULL),
				options_(NULL),
				packet_(NULL),
				videoindex_(-1),
				codec_id_(AV_CODEC_ID_NONE)
				{
		av_register_all();
		avformat_network_init();
	}

	~RtspHisiMppImpl() {
		av_free(packet_);
		avformat_close_input(&pFormatCtx_);
	}

	int init();
	bool open(const std::string &rtsp_addr) {}

	void reopen() {}

	bool read(cv::Mat &image);
	bool read(FILE* fou);
	void set(int option, int value) {}
	bool isOpened() {}
	void release() {}

	int getFrame(cv::Mat &image);

	void restart();

	void sendh264Stream();

	void stop();

 private:
    int inited_;
	AVFormatContext *pFormatCtx_;
	AVDictionary *options_;
	int videoindex_;
	AVPacket *packet_;
	enum AVCodecID codec_id_;
	std::shared_ptr<Vdec> vde_;
	
	

    int64_t last_decode_time_;
	const int mpp_exception_decode_time_window_ = 5 * 60 * 1000;
	const int decode_gop_time_window_ = 60 * 1000;
};


