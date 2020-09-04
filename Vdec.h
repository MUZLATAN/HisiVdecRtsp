//
// Created by z on 2020/8/12
//
#pragma once

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

extern "C" {
#include <libavformat/avformat.h>
};

#include "hi_type.h"
#include "hi_defines.h"
#include "mpi_vo.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#include "mpi_hdmi.h"
#include "hi_comm_vo.h"
#include "hi_buffer.h"
#include "hi_comm_vdec.h"
#include "hi_comm_video.h"
#include "hi_comm_hdmi.h"
#include "hi_mipi_tx.h"
#include "hi_common.h"


#define VO_DEV_UHD  0 /* VO's ultra HD device:HD0 */

typedef enum hiPIC_SIZE_S
{
    PCIF,
    P360P,      /* 640 * 360 */
    PD1_PAL,    /* 720 * 576 */
    PD1_NTSC,   /* 720 * 480 */
    P720P,      /* 1280 * 720  */
    P1080P,     /* 1920 * 1080 */
    PBUTT
} PIC_SIZE_S;

typedef enum hiTHREAD_CONTRL_S
{
    THREAD_CONTROL_START,
    THREAD_CONTROL_PAUSE,
    THREAD_CONTROL_STOP,
}THREAD_CONTRL_S;


typedef struct hiVDEC_THREAD_PARAMS
{
    HI_S32 s32ChnId;
    PAYLOAD_TYPE_E enType;
    HI_CHAR cFilePath[128];
    HI_CHAR cFileName[128];
    HI_S32 s32StreamMode;
    HI_S32 s32MilliSec;
    HI_S32 s32MinBufSize;
    HI_S32 s32IntervalTime;
    THREAD_CONTRL_S eThreadCtrl;
    HI_U64  u64PtsInit;
    HI_U64  u64PtsIncrease;
    HI_BOOL bCircleSend;
}VDEC_THREAD_PARAMS;

typedef struct hiVDEC_VIDEO_ATTR
{
    VIDEO_DEC_MODE_E enDecMode;
    HI_U32              u32RefFrameNum;
    DATA_BITWIDTH_E  enBitWidth;
}VDEC_VIDEO_ATTR;

typedef struct hiVDEC_PICTURE_ATTR
{
    PIXEL_FORMAT_E enPixelFormat;
    HI_U32         u32Alpha;
}VDEC_PICTURE_ATTR;



typedef struct hiVDEC_ATTR
{
    PAYLOAD_TYPE_E enType;
    VIDEO_MODE_E   enMode;
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U32 u32FrameBufCnt;
    HI_U32 u32DisplayFrameNum;
    union
    {
        VDEC_VIDEO_ATTR stSapmleVdecVideo;      /* structure with video ( h265/h264) */
        VDEC_PICTURE_ATTR stSapmleVdecPicture; /* structure with picture (jpeg/mjpeg )*/
    };
}VDEC_ATTR;

typedef enum hiVO_MODE_E
{
    VO_MOD_1MUX  ,
    VO_MOD_2MUX  ,
    VO_MOD_4MUX  ,
    VO_MOD_8MUX  ,
    VO_MOD_9MUX  ,
    VO_MOD_16MUX ,
    VO_MOD_25MUX ,
    VO_MOD_36MUX ,
    VO_MOD_49MUX ,
    VO_MOD_64MUX ,
    VO_MOD_2X4   ,
    VO_MOD_BUTT
} VO_MODE_E;
typedef struct hiVO_CONFIG_S
{
    /* for device */
    VO_DEV                  VoDev;
    VO_INTF_TYPE_E          enVoIntfType;
    VO_INTF_SYNC_E          enIntfSync;
    PIC_SIZE_S              enPicSize;
    HI_U32                  u32BgColor;

    /* for layer */
    PIXEL_FORMAT_E          enPixFormat;
    RECT_S                  stDispRect;
    SIZE_S                  stImageSize;
    VO_PART_MODE_E          enVoPartMode;

    HI_U32                  u32DisBufLen;
    DYNAMIC_RANGE_E         enDstDynamicRange;

    /* for chnnel */
    VO_MODE_E        enVoMode;
} VO_CONFIG_S;

typedef struct hiVDEC_BUF
{
    HI_U32  u32PicBufSize;
    HI_U32  u32TmvBufSize;
    HI_BOOL bPicBufAlloc;
    HI_BOOL bTmvBufAlloc;
}VDEC_BUF;

typedef struct hiVO_CONFIGS
{
    /* for device */
    VO_DEV                  VoDev;
    VO_INTF_TYPE_E          enVoIntfType;
    VO_INTF_SYNC_E          enIntfSync;
    PIC_SIZE_S              enPicSize;
    HI_U32                  u32BgColor;

    /* for layer */
    PIXEL_FORMAT_E          enPixFormat;
    RECT_S                  stDispRect;
    SIZE_S                  stImageSize;
    VO_PART_MODE_E          enVoPartMode;

    HI_U32                  u32DisBufLen;
    DYNAMIC_RANGE_E         enDstDynamicRange;

    /* for chnnel */
    VO_MODE_E        enVoMode;
} VO_CONFIGS;


struct vdec_para{
	VDEC_THREAD_PARAMS vdethpara;
	AVFormatContext *pFCtx_;
    AVPacket *packet_;
	pthread_t vdecth;
    int videoIdx;
};

class Vdec
{
    public:
        Vdec():g_mVdecVBSource(VB_SOURCE_MODULE)
        {

        }
        ~Vdec()
        {
            // VDEC_Stop(1);
            // VDEC_ExitVBPool();
        }


    public:
        HI_S32 GetPicSize(PIC_SIZE_S enPicSize, SIZE_S* pstSize);

        HI_S32 SYS_Init(VB_CONFIG_S* pstVbConfig);
        
        HI_S32 VDEC_InitVBPool(HI_U32 ChnNum, VDEC_ATTR *pastSampleVdec);

        HI_S32 VDEC_Start(HI_S32 s32ChnNum, VDEC_ATTR *pastSampleVdec);

        HI_S32 init();


        HI_S32 VDEC_Stop(HI_S32 s32ChnNum);

        HI_VOID VDEC_ExitVBPool(HI_VOID);

        HI_VOID SYS_Exit(void);

        HI_VOID VDEC_StartSendStream(HI_S32 s32ChnNum, VDEC_THREAD_PARAMS *pstVdecSend, pthread_t *pVdecThread);

        static HI_VOID * VDEC_SendStream(HI_VOID *pArgs);

        static HI_VOID * VDEC_SendH264Stream(HI_VOID *pArgs);

        HI_VOID VDEC_StopSendStream(HI_S32 s32ChnNum, VDEC_THREAD_PARAMS *pstVdecSend, pthread_t *pVdecThread);

        HI_VOID VDEC_CmdCtrl(HI_S32 s32ChnNum,VDEC_THREAD_PARAMS *pstVdecSend, pthread_t *pVdecThread);

        HI_S32 UnBind_VPSS();

        int getVpssGrp(){return VpssGrp;}
        int getVpssChn(){return VpssChn;}

    public:
        vdec_para vdepara_;

    private:
        VB_SOURCE_E  g_mVdecVBSource;
        VB_POOL g_ahPicVbPool[10] = {(-1U), (-1U), (-1U),(-1U),(-1U),(-1U),(-1U),(-1U),(-1U),(-1U)};
        VB_POOL g_ahTmvVbPool[10] = {(-1U), (-1U), (-1U),(-1U),(-1U),(-1U),(-1U),(-1U),(-1U),(-1U)};

        VDEC_ATTR astSampleVdec;

        MPP_CHN_S stSrcChn;
        MPP_CHN_S stDestChn;
        VPSS_GRP VpssGrp = 2;
        VPSS_CHN VpssChn = 1;

        
};