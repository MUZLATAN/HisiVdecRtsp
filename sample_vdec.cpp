
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <memory>
#include "Vdec.h"

#include "HisiRtsp.h"

#define SAMPLE_STREAM_PATH "/mnt/embed/myvdec/source_file"




#ifndef __HuaweiLite__
HI_VOID SAMPLE_VDEC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo)
    {
        HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
        HI_MPI_VB_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}
#endif

/******************************************************************************
* function    : main()
* Description : video vdec sample
******************************************************************************/
#ifdef __HuaweiLite__
    int app_main(int argc, char *argv[])
#else
    int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret = HI_SUCCESS;



#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_VDEC_HandleSig);
    signal(SIGTERM, SAMPLE_VDEC_HandleSig);
#endif


    // s32Ret = SAMPLE_H264_VDEC_VPSS_VO();

    RtspHisiMppImpl rtsp_impl_("rtsp://admin:admin@192.168.192.72:554/h264/ch30/main/av_stream") ;

    std::cout<<"rtsp_impl_ init ..."<<std::endl;
    rtsp_impl_.init();
    std::cout<<"while true .."<<std::endl;

    FILE* fy = fopen("naoutput.yuv", "wb+");
    int i = 0;
    while (++i < 100)
    {
        cv::Mat image;
        if (!rtsp_impl_.read(fy))
        {
            std::cout<<"read one image"<<std::endl;
        }
    }
    printf("read finished !\n");
    rtsp_impl_.stop();
            

    return s32Ret;
}


