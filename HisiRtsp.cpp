//
// Created by z on 2020/8/12
//

#include "HisiRtsp.h"
// #include "g//LOG///LOGging.h"

// #include <folly/json.h>
#include <unistd.h>

int RtspHisiMppImpl::init() {
    std::cout<<"rtsp init func"<<std::endl;

    inited_ = false;
	if (pFormatCtx_) {
		avformat_free_context(pFormatCtx_);
	}
	if (packet_) {
		av_free(packet_);
	}
	

	//设置缓存大小,1080p可将值跳到最大
	av_dict_set(&options_, "buffer_size", "1024000", 0);
	// tcp open
	av_dict_set(&options_, "rtsp_transport", "tcp", 0);
	// set timeout unit: us
	av_dict_set(&options_, "stimeout", "5000000", 0);
	// set max delay
	av_dict_set(&options_, "max_delay", "500000", 0);

    std::cout<<" avformat_alloc_context"<<std::endl;
	pFormatCtx_ = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx_, rtsp_addr_.c_str(), NULL, &options_) !=
			0) {
        //LOG(ERROR) << "Couldn't open input stream.";
		return -1;
	}

	// get video stream info
	if (avformat_find_stream_info(pFormatCtx_, NULL) < 0) {
		//LOG(ERROR) << "Couldn't find stream information.";
		return -1;
	}

	// find stream video codec type
	unsigned int i = 0;
	for (i = 0; i < pFormatCtx_->nb_streams; i++) {
		if (pFormatCtx_->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex_ = i;
			codec_id_ = pFormatCtx_->streams[i]->codec->codec_id;
			break;
		}
	}

	if (videoindex_ == -1) {
		return -1;
	}

	packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));

	// if (AV_CODEC_ID_H264 == codec_id_) {
	// 	mppdec_->setMppDecodeType(MPP_VIDEO_CodingAVC);
	// 	//LOG(INFO) << "video stream decode type is h264";
	// } else if (AV_CODEC_ID_HEVC == codec_id_) {
	// 	mppdec_->setMppDecodeType(MPP_VIDEO_CodingHEVC);
	// 		//LOG(INFO) << codec_id_ << " ,video stream decode type is h265";
	// }

	// if (ret == 0) {
	// 	inited_ = true;
	// }

	// last_decode_time_ = SysPublicTool::getCurrentTime();


	printf(" vedc init here!\n");
	vde_ = std::make_unique<Vdec>();
	vde_->init();
	vde_->vdepara_.vdecth = 0;
	vde_->vdepara_.pFCtx_ = pFormatCtx_;
	vde_->vdepara_.videoIdx = videoindex_;
	vde_->vdepara_.packet_ = packet_;

	
    vde_->vdepara_.vdethpara.enType          = PT_H264;
    vde_->vdepara_.vdethpara.s32StreamMode   = VIDEO_MODE_FRAME;
    vde_->vdepara_.vdethpara.s32ChnId        = 0;
    vde_->vdepara_.vdethpara.s32IntervalTime = 1000;
    vde_->vdepara_.vdethpara.u64PtsInit      = 0;
    vde_->vdepara_.vdethpara.u64PtsIncrease  = 0;
    vde_->vdepara_.vdethpara.eThreadCtrl     = THREAD_CONTROL_START;
    vde_->vdepara_.vdethpara.bCircleSend     = HI_TRUE;
    vde_->vdepara_.vdethpara.s32MilliSec     = 0;
    vde_->vdepara_.vdethpara.s32MinBufSize   = (1920 * 1080 * 3)>>1;

	// 新开启的一个线程 send stream
	printf("send stream here\n");
	sendh264Stream();


	return 0;
}

void RtspHisiMppImpl::sendh264Stream(){

	pthread_create(&vde_->vdepara_.vdecth, 0, Vdec::VDEC_SendStream, (HI_VOID *)&vde_->vdepara_);
    

}

void RtspHisiMppImpl::stop(){
	
	vde_->vdepara_.vdethpara.eThreadCtrl = THREAD_CONTROL_STOP;

    HI_MPI_VDEC_StopRecvStream(0);
    if(0 == vde_->vdepara_.vdecth)
    {
        pthread_join(vde_->vdepara_.vdecth, HI_NULL);
        vde_->vdepara_.vdecth = 0;
    }

	vde_->SYS_Exit();

    avformat_close_input(&pFormatCtx_);
}

void RtspHisiMppImpl::restart() {
	// folly::dynamic root = folly::dynamic::object;
	// folly::dynamic data = folly::dynamic::object;
	// data["camera_sn"] = ServiceVariableManager::Get("client_sn");
	// data["time"] = SysPublicTool::getCurrentTime();
	// data["action"] = static_cast<int>(k//LOGMessageMonitor);
	// data["cv_message"] = "mpp decode failed, now restart!";
	// root["name"] = "algo-metric";
	// root["data"] = data;
	// std::string payload = folly::toJson(root);
	//LOG(INFO) << "version metirc action message: " << payload;
	// std::string response;
	// int ret = HttpRequestClient::chunnel(payload, response);

	// if (!ret) {
	// 	//LOG(INFO) << "mpp decode failed, now restart it after monitor message: "
	// 						<< payload;
	// 	sync();
	// 	system("reboot");
	// }
}

bool RtspHisiMppImpl::read(FILE* fou){
	int vdec_chn = 0;
	VIDEO_FRAME_INFO_S stFrame;
	int s32Ret;
	int s32MilliSec = 1000;
	int i = 0;
	
	s32Ret = HI_MPI_VPSS_GetChnFrame(vde_->getVpssGrp(), vde_->getVpssChn(), &stFrame, s32MilliSec);
    if (s32Ret != HI_SUCCESS){
		printf("read a frame failed %x\n", s32Ret);
		return false;
	}
        

    // printf(" w and h are: %d   %d\n", stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height);
    int u32Size = stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height * 3 / 2;
	int ylen = stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height;
	HI_VOID* yAddr =
		(unsigned char*)HI_MPI_SYS_Mmap(stFrame.stVFrame.u64PhyAddr[0], u32Size);

    fwrite(yAddr, ylen, 1, fou);
    int uvsize = stFrame.stVFrame.u32Stride[1] * stFrame.stVFrame.u32Height / 2;
	HI_VOID* uvAddr = yAddr + ylen;
    
	fwrite(uvAddr, uvsize, 1, fou);

    s32Ret = HI_MPI_VPSS_ReleaseChnFrame(vde_->getVpssGrp(), vde_->getVpssChn(), &stFrame);
	if (s32Ret != HI_SUCCESS) {
		printf("HI_MPI_VPSS_ReleaseChnFrame error %x \n", s32Ret);
	}
    
	s32Ret = HI_MPI_SYS_Munmap(yAddr, u32Size);
	if (s32Ret != HI_SUCCESS) {
		printf("HI_MPI_SYS_Munmap error %x \n", s32Ret);
	}
	return true;
}