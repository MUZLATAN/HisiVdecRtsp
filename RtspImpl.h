//
// Created by z on 2020/8/12
//

#pragma once

#include <string>
#include <opencv2/opencv.hpp>
#include <memory>
// #include "src/include/Callback.h"



class RtspImpl {
 public:
	RtspImpl(const std::string &rtsp_addr) : rtsp_addr_(rtsp_addr) {}
	virtual ~RtspImpl() {}
	virtual int init() {}
	virtual bool read(cv::Mat &image) {}
	virtual bool open(const std::string &rtsp_addr) {}
	virtual void release(){};
	virtual void set(int option, int value) {}
	virtual bool isOpened() {}
	// virtual void setDataCallback(const rtsp_data_callback_t &cb) {}

 protected:
	std::string rtsp_addr_;
	bool opened_;
};

// class RtspCVImpl : public RtspImpl {
//  public:
// 	RtspCVImpl(const std::string &rtsp_addr) : RtspImpl(rtsp_addr) {}
// 	~RtspCVImpl() { release(); }

// 	virtual int init() {
// 		capture_ = std::make_unique<cv::VideoCapture>(rtsp_addr_);
// 		return 0;
// 	}

// 	virtual bool read(cv::Mat &image) {
// 		opened_ = capture_->read(image);
// 		// if (cb_) {
// 		// 	cb_(image);
// 		// }
// 		return opened_;
// 	}
// 	virtual bool open(const std::string &rtsp_addr) {}
// 	virtual void release() {
// 		if (capture_) {
// 			capture_->release();
// 		}
// 	};
// 	virtual bool isOpened() { return opened_; }
// 	// virtual void setDataCallback(const rtsp_data_callback_t &cb) { cb_ = cb; }
// 	virtual void set(int option, int value) { capture_->set(option, value); }

//  private:
// 	std::unique_ptr<cv::VideoCapture> capture_;
// 	// rtsp_data_callback_t cb_;
// };

