//
// Created by z on 2020/8/12
//
#include "Vdec.h"

/******************************************************************************
* function : get picture size(w*h), according enPicSize
******************************************************************************/
HI_S32 Vdec::GetPicSize(PIC_SIZE_S enPicSize, SIZE_S* pstSize)
{
    switch (enPicSize)
    {
        case PCIF:   /* 352 * 288 */
            pstSize->u32Width  = 352;
            pstSize->u32Height = 288;
            break;

        case P360P:   /* 640 * 360 */
            pstSize->u32Width  = 640;
            pstSize->u32Height = 360;
            break;

        case PD1_PAL:   /* 720 * 576 */
            pstSize->u32Width  = 720;
            pstSize->u32Height = 576;
            break;

        case PD1_NTSC:   /* 720 * 480 */
            pstSize->u32Width  = 720;
            pstSize->u32Height = 480;
            break;

        case P720P:   /* 1280 * 720 */
            pstSize->u32Width  = 1280;
            pstSize->u32Height = 720;
            break;

        case P1080P:  /* 1920 * 1080 */
            pstSize->u32Width  = 1920;
            pstSize->u32Height = 1080;
            break;
        default:
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/
HI_S32 Vdec::SYS_Init(VB_CONFIG_S* pstVbConfig)
{
    HI_S32 s32Ret = HI_FAILURE;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    if (NULL == pstVbConfig)
    {
        printf("input parameter is null, it is invaild!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_SetConfig(pstVbConfig);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Init failed!\n");
        HI_MPI_VB_Exit();
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Vdec::VDEC_InitVBPool(HI_U32 ChnNum, VDEC_ATTR *pastSampleVdec)
{
    VB_CONFIG_S stVbConf;
    HI_S32 i, j, pos=0, s32Ret;
    HI_BOOL bFindFlag;
    VDEC_BUF astSampleVdecBuf[VDEC_MAX_CHN_NUM];
    VB_POOL_CONFIG_S stVbPoolCfg;

    memset(astSampleVdecBuf, 0, sizeof(VDEC_BUF)*VDEC_MAX_CHN_NUM);
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

    for(i=0; i<ChnNum; i++)
    {
        if(PT_H265 == pastSampleVdec[i].enType)
        {
            astSampleVdecBuf[i].u32PicBufSize = VDEC_GetPicBufferSize(pastSampleVdec[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height,
                                                    PIXEL_FORMAT_YVU_SEMIPLANAR_420, pastSampleVdec[i].stSapmleVdecVideo.enBitWidth, 0);
            astSampleVdecBuf[i].u32TmvBufSize = VDEC_GetTmvBufferSize(pastSampleVdec[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height);
        }
        else if(PT_H264 == pastSampleVdec[i].enType)
        {
            astSampleVdecBuf[i].u32PicBufSize = VDEC_GetPicBufferSize(pastSampleVdec[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height,
                                                    PIXEL_FORMAT_YVU_SEMIPLANAR_420, pastSampleVdec[i].stSapmleVdecVideo.enBitWidth, 0);
            if(VIDEO_DEC_MODE_IPB == pastSampleVdec[i].stSapmleVdecVideo.enDecMode)
            {
                astSampleVdecBuf[i].u32TmvBufSize = VDEC_GetTmvBufferSize(pastSampleVdec[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height);
            }
        }
        else
        {
            astSampleVdecBuf[i].u32PicBufSize = VDEC_GetPicBufferSize(pastSampleVdec[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height,
                                                    pastSampleVdec[i].stSapmleVdecPicture.enPixelFormat, DATA_BITWIDTH_8, 0);
        }
    }

    /* PicBuffer */
    for(j=0; j<VB_MAX_COMM_POOLS; j++)
    {
        bFindFlag = HI_FALSE;
        for(i=0; i<ChnNum; i++)
        {
            if((HI_FALSE == bFindFlag) && (0 != astSampleVdecBuf[i].u32PicBufSize) && (HI_FALSE == astSampleVdecBuf[i].bPicBufAlloc) )
            {
                stVbConf.astCommPool[j].u64BlkSize = astSampleVdecBuf[i].u32PicBufSize;
                stVbConf.astCommPool[j].u32BlkCnt  = pastSampleVdec[i].u32FrameBufCnt;
                astSampleVdecBuf[i].bPicBufAlloc   = HI_TRUE;
                bFindFlag                          = HI_TRUE;
                pos = j;
            }

            if((HI_TRUE == bFindFlag) && (HI_FALSE == astSampleVdecBuf[i].bPicBufAlloc)
                && (stVbConf.astCommPool[j].u64BlkSize == astSampleVdecBuf[i].u32PicBufSize) )
            {
                stVbConf.astCommPool[j].u32BlkCnt += pastSampleVdec[i].u32FrameBufCnt;
                astSampleVdecBuf[i].bPicBufAlloc   = HI_TRUE;
            }
        }
    }

    /* TmvBuffer */
    for(j=pos+1; j<VB_MAX_COMM_POOLS; j++)
    {
        bFindFlag = HI_FALSE;
        for(i=0; i<ChnNum; i++)
        {
            if((HI_FALSE == bFindFlag) && (0 != astSampleVdecBuf[i].u32TmvBufSize) && (HI_FALSE == astSampleVdecBuf[i].bTmvBufAlloc) )
            {
                stVbConf.astCommPool[j].u64BlkSize = astSampleVdecBuf[i].u32TmvBufSize;
                stVbConf.astCommPool[j].u32BlkCnt  = pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum+1;
                astSampleVdecBuf[i].bTmvBufAlloc   = HI_TRUE;
                bFindFlag                          = HI_TRUE;
                pos = j;
            }

            if((HI_TRUE == bFindFlag) && (HI_FALSE == astSampleVdecBuf[i].bTmvBufAlloc)
                && (stVbConf.astCommPool[j].u64BlkSize == astSampleVdecBuf[i].u32TmvBufSize) )
            {
                stVbConf.astCommPool[j].u32BlkCnt += pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum+1;
                astSampleVdecBuf[i].bTmvBufAlloc   = HI_TRUE;
            }
        }
    }
    stVbConf.u32MaxPoolCnt = pos + 1;

    if(VB_SOURCE_MODULE == g_mVdecVBSource)
    {
        HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
        HI_MPI_VB_SetModPoolConfig(VB_UID_VDEC, &stVbConf);
        s32Ret = HI_MPI_VB_InitModCommPool(VB_UID_VDEC);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VB_InitModCommPool fail for 0x%x\n", s32Ret);
            HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
            return HI_FAILURE;
        }
    }
    else if (VB_SOURCE_USER == g_mVdecVBSource)
    {
        for (i = 0; i < ChnNum; i++)
        {
            if ( (0 != astSampleVdecBuf[i].u32PicBufSize) && (0 != pastSampleVdec[i].u32FrameBufCnt))
            {
                memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
                stVbPoolCfg.u64BlkSize  = astSampleVdecBuf[i].u32PicBufSize;
                stVbPoolCfg.u32BlkCnt   = pastSampleVdec[i].u32FrameBufCnt;
                stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
                g_ahPicVbPool[i] = HI_MPI_VB_CreatePool(&stVbPoolCfg);
                if (VB_INVALID_POOLID == g_ahPicVbPool[i])
                {
                    goto fail;
                }
            }
            if (0 != astSampleVdecBuf[i].u32TmvBufSize)
            {
                memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
                stVbPoolCfg.u64BlkSize  = astSampleVdecBuf[i].u32TmvBufSize;
                stVbPoolCfg.u32BlkCnt   = pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum+1;
                stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
                g_ahTmvVbPool[i] = HI_MPI_VB_CreatePool(&stVbPoolCfg);
                if (VB_INVALID_POOLID == g_ahTmvVbPool[i])
                {
                    goto fail;
                }
            }
        }
    }

    return HI_SUCCESS;

    fail:
        for (;i>=0;i--)
        {
            if (VB_INVALID_POOLID != g_ahPicVbPool[i])
            {
                s32Ret = HI_MPI_VB_DestroyPool(g_ahPicVbPool[i]);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_MPI_VB_DestroyPool %d fail!\n",g_ahPicVbPool[i]);
                }
                g_ahPicVbPool[i] = VB_INVALID_POOLID;
            }
            if (VB_INVALID_POOLID != g_ahTmvVbPool[i])
            {
                s32Ret = HI_MPI_VB_DestroyPool(g_ahTmvVbPool[i]);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_MPI_VB_DestroyPool %d fail!\n",g_ahTmvVbPool[i]);
                }
                g_ahTmvVbPool[i] = VB_INVALID_POOLID;
            }
        }
        return HI_FAILURE;
}


HI_S32 Vdec::VDEC_Start(HI_S32 s32ChnNum, VDEC_ATTR *pastSampleVdec)
{
    HI_S32  i;
    VDEC_CHN_ATTR_S stChnAttr[VDEC_MAX_CHN_NUM];
    VDEC_CHN_POOL_S stPool;
    VDEC_CHN_PARAM_S stChnParam;
    VDEC_MOD_PARAM_S stModParam;

    HI_S32 ret = HI_MPI_VDEC_GetModParam(&stModParam);
    if (HI_SUCCESS != ret)
    {
        printf("vdec get mod param failed!\n");
        exit(0);
    }

    stModParam.enVdecVBSource = g_mVdecVBSource;
    ret = HI_MPI_VDEC_SetModParam(&stModParam);
    if (HI_SUCCESS != ret)
    {
        printf("vdec set mod param failed\n");
    }

    for(i=0; i<s32ChnNum; i++)
    {
        stChnAttr[i].enType           = pastSampleVdec[i].enType;
        stChnAttr[i].enMode           = pastSampleVdec[i].enMode;
        stChnAttr[i].u32PicWidth      = pastSampleVdec[i].u32Width;
        stChnAttr[i].u32PicHeight     = pastSampleVdec[i].u32Height;
        stChnAttr[i].u32StreamBufSize = pastSampleVdec[i].u32Width*pastSampleVdec[i].u32Height;
        stChnAttr[i].u32FrameBufCnt   = pastSampleVdec[i].u32FrameBufCnt;

        if (PT_H264 == pastSampleVdec[i].enType || PT_H265 == pastSampleVdec[i].enType)
        {
            stChnAttr[i].stVdecVideoAttr.u32RefFrameNum     = pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum;
            stChnAttr[i].stVdecVideoAttr.bTemporalMvpEnable = HI_TRUE;
            if (PT_H264 == pastSampleVdec[i].enType)
            {
                stChnAttr[i].stVdecVideoAttr.bTemporalMvpEnable = HI_FALSE;
            }
            stChnAttr[i].u32FrameBufSize  = VDEC_GetPicBufferSize(stChnAttr[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height,
                    PIXEL_FORMAT_YVU_SEMIPLANAR_420, pastSampleVdec[i].stSapmleVdecVideo.enBitWidth, 0);
        }
        else if (PT_JPEG == pastSampleVdec[i].enType || PT_MJPEG == pastSampleVdec[i].enType)
        {
            stChnAttr[i].enMode           = VIDEO_MODE_FRAME;
            stChnAttr[i].u32FrameBufSize  = VDEC_GetPicBufferSize(stChnAttr[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height,
                                                pastSampleVdec[i].stSapmleVdecPicture.enPixelFormat, DATA_BITWIDTH_8, 0);
        }

        ret = HI_MPI_VDEC_CreateChn(i, &stChnAttr[i]);
        if (HI_SUCCESS != ret)
        {
            printf("HI_MPI_VDEC_CreateChn failed \n");

        }

        if (VB_SOURCE_USER == g_mVdecVBSource)
        {
            stPool.hPicVbPool = g_ahPicVbPool[i];
            stPool.hTmvVbPool = g_ahTmvVbPool[i];
            ret = HI_MPI_VDEC_AttachVbPool(i, &stPool);
            if (HI_SUCCESS != ret)
            {
                printf("HI_MPI_VDEC_AttachVbPool failed \n");

            }
        }

        ret = HI_MPI_VDEC_GetChnParam(i, &stChnParam);
        if (HI_SUCCESS != ret)
        {
            printf("HI_MPI_VDEC_GetChnParam failed \n");

        }
        if (PT_H264 == pastSampleVdec[i].enType || PT_H265 == pastSampleVdec[i].enType)
        {
            stChnParam.stVdecVideoParam.enDecMode         = pastSampleVdec[i].stSapmleVdecVideo.enDecMode;
            stChnParam.stVdecVideoParam.enCompressMode    = COMPRESS_MODE_NONE;
            stChnParam.stVdecVideoParam.enVideoFormat     = VIDEO_FORMAT_TILE_64x16;
            if(VIDEO_DEC_MODE_IPB == stChnParam.stVdecVideoParam.enDecMode)
            {
                stChnParam.stVdecVideoParam.enOutputOrder = VIDEO_OUTPUT_ORDER_DISP;
            }
            else
            {
                stChnParam.stVdecVideoParam.enOutputOrder = VIDEO_OUTPUT_ORDER_DEC;
            }
        }
        else
        {
            stChnParam.stVdecPictureParam.enPixelFormat   = pastSampleVdec[i].stSapmleVdecPicture.enPixelFormat;
            stChnParam.stVdecPictureParam.u32Alpha        = pastSampleVdec[i].stSapmleVdecPicture.u32Alpha;
        }
        stChnParam.u32DisplayFrameNum                     = pastSampleVdec[i].u32DisplayFrameNum;
        ret = HI_MPI_VDEC_SetChnParam(i, &stChnParam);
        if (HI_SUCCESS != ret)
        {
            printf("HI_MPI_VDEC_GetChnParam failed \n");
        }

        ret = HI_MPI_VDEC_StartRecvStream(i);
        if (HI_SUCCESS != ret)
        {
            printf("HI_MPI_VDEC_StartRecvStream failed \n");
        }
        
    }

    return HI_SUCCESS;
}




HI_S32 Vdec::VDEC_Stop(HI_S32 s32ChnNum)
{
    HI_S32 i;

    for(i=0; i<s32ChnNum; i++)
    {
        HI_MPI_VDEC_StopRecvStream(i);
        HI_MPI_VDEC_DestroyChn(i);
    }

    return HI_SUCCESS;
}

HI_VOID Vdec::VDEC_ExitVBPool(HI_VOID)
{
    HI_S32 i, s32Ret;

    if(VB_SOURCE_MODULE == g_mVdecVBSource)
    {
        HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
    }
    else if (VB_SOURCE_USER == g_mVdecVBSource)
    {
        for (i=VB_MAX_POOLS-1; i>=0; i--)
        {
            if (VB_INVALID_POOLID != g_ahPicVbPool[i])
            {
                s32Ret = HI_MPI_VB_DestroyPool(g_ahPicVbPool[i]);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_MPI_VB_DestroyPool %d fail!\n",g_ahPicVbPool[i]);
                }
                g_ahPicVbPool[i] = VB_INVALID_POOLID;
            }
            if (VB_INVALID_POOLID != g_ahTmvVbPool[i])
            {
                s32Ret = HI_MPI_VB_DestroyPool(g_ahTmvVbPool[i]);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_MPI_VB_DestroyPool %d fail!\n",g_ahTmvVbPool[i]);
                }
                g_ahTmvVbPool[i] = VB_INVALID_POOLID;
            }
        }
    }

    return;
}

HI_S32 Vdec::UnBind_VPSS()
{


    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = VpssChn;

    HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);

    return HI_SUCCESS;
}

HI_VOID Vdec::SYS_Exit(void)
{
    UnBind_VPSS();

    VDEC_Stop(1);
    VDEC_ExitVBPool();

    HI_MPI_VB_Exit();
    return;
}


HI_VOID Vdec::VDEC_CmdCtrl(HI_S32 s32ChnNum,VDEC_THREAD_PARAMS *pstVdecSend, pthread_t *pVdecThread)
{
    HI_S32 i, s32Ret;
    VDEC_CHN_STATUS_S stStatus;
    char c=0;

    for(i=0; i<s32ChnNum; i++)
    {
        if(HI_TRUE == pstVdecSend[i].bCircleSend)
        {
            goto WHILE;
        }
    }

    printf("decoding..............");
    for(i=0; i<s32ChnNum; i++)
    {
        if(0 != pVdecThread[i])
        {
            s32Ret = pthread_join(pVdecThread[i], HI_NULL);
            if(0 == s32Ret)
            {
                pVdecThread[i] = 0;
            }
        }
        pVdecThread[i] = 0;
        while(1)
        {
            s32Ret = HI_MPI_VDEC_QueryStatus(pstVdecSend[i].s32ChnId, &stStatus);
            if(s32Ret != HI_SUCCESS)
            {
                printf("chn %d HI_MPI_VDEC_QueryStatus fail!!!\n",s32Ret);
                return;
            }
            if((0 == stStatus.u32LeftStreamBytes)&&(0 == stStatus.u32LeftPics))
            {
                // PRINTF_VDEC_CHN_STATUS(pstVdecSend[i].s32ChnId, stStatus);
                break;
            }
            usleep(1000);
        }
    }
    printf("end!\n");
    return;

WHILE:
    while(1)
    {
        printf("\nSAMPLE_TEST:press 'e' to exit; 'q' to query;\n");
        c = getchar();

        if (c == 'e')
            break;        
        else if (c == 'q')
        {
            for (i=0; i<s32ChnNum; i++)
            {
                HI_MPI_VDEC_QueryStatus(pstVdecSend[i].s32ChnId, &stStatus);
                // PRINTF_VDEC_CHN_STATUS(pstVdecSend[i].s32ChnId, stStatus);
            }
        }
    }
    return;
}

HI_VOID Vdec::VDEC_StopSendStream(HI_S32 s32ChnNum, VDEC_THREAD_PARAMS *pstVdecSend, pthread_t *pVdecThread)
{
    HI_S32  i;

    for(i=0; i<s32ChnNum; i++)
    {
        pstVdecSend[i].eThreadCtrl = THREAD_CONTROL_STOP;
        HI_MPI_VDEC_StopRecvStream(i);
        if(0 != pVdecThread[i])
        {
            pthread_join(pVdecThread[i], HI_NULL);
            pVdecThread[i] = 0;
        }
    }
}

HI_VOID Vdec::VDEC_StartSendStream(HI_S32 s32ChnNum, VDEC_THREAD_PARAMS *pstVdecSend, pthread_t *pVdecThread)
{
    HI_S32  i;

    for(i=0; i<s32ChnNum; i++)
    {
        pVdecThread[i] = 0;
        pthread_create(&pVdecThread[i], 0, Vdec::VDEC_SendStream, (HI_VOID *)&pstVdecSend[i]);
    }
}


HI_VOID * Vdec::VDEC_SendStream(HI_VOID *pArgs)
{

    vdec_para* pvde_param = (vdec_para*)pArgs;
    VDEC_THREAD_PARAMS pstVdecThreadParam =pvde_param->vdethpara;
    HI_BOOL bEndOfStream = HI_FALSE;
    HI_S32 s32UsedBytes = 0, s32ReadLen = 0;
    HI_U8 *pu8Buf;
    VDEC_STREAM_S stStream;
    HI_BOOL bFindStart, bFindEnd;
    HI_U64 u64PTS = 0;
    HI_U32 u32Len, u32Start;
    HI_S32 s32Ret,  i;
    HI_CHAR cStreamFile[256];

    prctl(15, "VideoSendStream", 0,0,0);


    

    pu8Buf = (HI_U8*)malloc(pstVdecThreadParam.s32MinBufSize);
    if(pu8Buf == NULL)
    {
        printf("chn %d can't alloc %d in send stream thread!\n", pstVdecThreadParam.s32ChnId, pstVdecThreadParam.s32MinBufSize);
        // fclose(fpStrm);
        return (HI_VOID *)(HI_FAILURE);
    }
    fflush(stdout);

    AVFormatContext * pfctx = pvde_param->pFCtx_;
    AVPacket *pck_ = pvde_param->packet_ ;
    // FILE *ffm = fopen("out.h264", "wb+");

    u64PTS = pstVdecThreadParam.u64PtsInit;
    while (1)
    {
        if (pstVdecThreadParam.eThreadCtrl == THREAD_CONTROL_STOP)
        {
            break;
        }
        else if (pstVdecThreadParam.eThreadCtrl == THREAD_CONTROL_PAUSE)
        {
            sleep(1);
            continue;
        }

        //read data from ffmpeg rtsp
        if (av_read_frame(pfctx, pck_) < 0)
            continue;

        if (pck_->stream_index != pvde_param->videoIdx){
            av_packet_unref(pck_);
            continue;
        }
            

        bEndOfStream = HI_FALSE;
        bFindStart   = HI_FALSE;
        bFindEnd     = HI_FALSE;
        u32Start     = 0;
   
        memcpy(pu8Buf, pck_->data, pck_->size);
        s32ReadLen = pck_->size;
        av_packet_unref(pck_);



        if (pstVdecThreadParam.s32StreamMode==VIDEO_MODE_FRAME && pstVdecThreadParam.enType == PT_H264)
        {
            for (i=0; i<s32ReadLen-8; i++)
            {
                int tmp = pu8Buf[i+3] & 0x1F;
                if (  pu8Buf[i    ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 &&
                       (
                           ((tmp == 0x5 || tmp == 0x1) && ((pu8Buf[i+4]&0x80) == 0x80)) ||
                           (tmp == 20 && (pu8Buf[i+7]&0x80) == 0x80)
                        )
                   )
                {
                    bFindStart = HI_TRUE;
                    i += 8;
                    break;
                }
            }

            for (; i<s32ReadLen-8; i++)
            {
                int tmp = pu8Buf[i+3] & 0x1F;
                if (  pu8Buf[i    ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 &&
                            (
                                  tmp == 15 || tmp == 7 || tmp == 8 || tmp == 6 ||
                                  ((tmp == 5 || tmp == 1) && ((pu8Buf[i+4]&0x80) == 0x80)) ||
                                  (tmp == 20 && (pu8Buf[i+7]&0x80) == 0x80)
                              )
                   )
                {
                    bFindEnd = HI_TRUE;
                    break;
                }
            }

            if(i>0)s32ReadLen = i;
            if (bFindStart == HI_FALSE)
            {
                printf("chn %d can not find H264 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                    pstVdecThreadParam.s32ChnId, s32ReadLen, s32UsedBytes);
            }
            if (bFindEnd == HI_FALSE)
            {
                s32ReadLen = i+8;
            }

        }
        else if (pstVdecThreadParam.s32StreamMode==VIDEO_MODE_FRAME
            && pstVdecThreadParam.enType == PT_H265)
        {
            HI_BOOL  bNewPic = HI_FALSE;
            for (i=0; i<s32ReadLen-6; i++)
            {
                HI_U32 tmp = (pu8Buf[i+3]&0x7E)>>1;
                bNewPic = (HI_BOOL)( pu8Buf[i+0] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1
                            && (tmp >= 0 && tmp <= 21) && ((pu8Buf[i+5]&0x80) == 0x80) );

                if (bNewPic)
                {
                    bFindStart = HI_TRUE;
                    i += 6;
                    break;
                }
            }

            for (; i<s32ReadLen-6; i++)
            {
                HI_U32 tmp = (pu8Buf[i+3]&0x7E)>>1;
                bNewPic = (HI_BOOL)(pu8Buf[i+0] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1
                            &&( tmp == 32 || tmp == 33 || tmp == 34 || tmp == 39 || tmp == 40 || ((tmp >= 0 && tmp <= 21) && (pu8Buf[i+5]&0x80) == 0x80) )
                             );

                if (bNewPic)
                {
                    bFindEnd = HI_TRUE;
                    break;
                }
            }
            if(i>0)s32ReadLen = i;

            if (bFindStart == HI_FALSE)
            {
                printf("chn %d can not find H265 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                    pstVdecThreadParam.s32ChnId, s32ReadLen, s32UsedBytes);
            }
            if (bFindEnd == HI_FALSE)
            {
                s32ReadLen = i+6;
            }

        }
        else if (pstVdecThreadParam.enType == PT_MJPEG || pstVdecThreadParam.enType == PT_JPEG)
        {
            for (i=0; i<s32ReadLen-1; i++)
            {
                if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8)
                {
                    u32Start = i;
                    bFindStart = HI_TRUE;
                    i = i + 2;
                    break;
                }
            }

            for (; i<s32ReadLen-3; i++)
            {
                if ((pu8Buf[i] == 0xFF) && (pu8Buf[i+1]& 0xF0) == 0xE0)
                {
                     u32Len = (pu8Buf[i+2]<<8) + pu8Buf[i+3];
                     i += 1 + u32Len;
                }
                else
                {
                    break;
                }
            }

            for (; i<s32ReadLen-1; i++)
            {
                if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD9)
                {
                    bFindEnd = HI_TRUE;
                    break;
                }
            }
            s32ReadLen = i+2;

            if (bFindStart == HI_FALSE)
            {
                printf("chn %d can not find JPEG start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                    pstVdecThreadParam.s32ChnId, s32ReadLen, s32UsedBytes);
            }
        }
        else
        {
            if((s32ReadLen != 0) && (s32ReadLen < pstVdecThreadParam.s32MinBufSize))
            {
                bEndOfStream = HI_TRUE;
            }
        }

        stStream.u64PTS       = u64PTS;
        stStream.pu8Addr      = pu8Buf + u32Start;
        stStream.u32Len       = s32ReadLen;
        stStream.bEndOfFrame  = (pstVdecThreadParam.s32StreamMode==VIDEO_MODE_FRAME)? HI_TRUE: HI_FALSE;
        stStream.bEndOfStream = bEndOfStream;
        stStream.bDisplay     = HI_TRUE;

SendAgain:
        s32Ret=HI_MPI_VDEC_SendStream(pstVdecThreadParam.s32ChnId, &stStream, pstVdecThreadParam.s32MilliSec);
        if( (HI_SUCCESS != s32Ret) && (THREAD_CONTROL_START == pstVdecThreadParam.eThreadCtrl) )
        {
            usleep(pstVdecThreadParam.s32IntervalTime);
            goto SendAgain;
        }
        else
        {
            bEndOfStream = HI_FALSE;
            s32UsedBytes = s32UsedBytes +s32ReadLen + u32Start;
            u64PTS += pstVdecThreadParam.u64PtsIncrease;
        }
        usleep(pstVdecThreadParam.s32IntervalTime);
    }

    /* send the flag of stream end */
    memset(&stStream, 0, sizeof(VDEC_STREAM_S) );
    stStream.bEndOfStream = HI_TRUE;
    HI_MPI_VDEC_SendStream(pstVdecThreadParam.s32ChnId, &stStream, -1);

    printf("\033[0;35m chn %d send steam thread return ...  \033[0;39m\n", pstVdecThreadParam.s32ChnId);
    fflush(stdout);
    if (pu8Buf != HI_NULL)
    {
        free(pu8Buf);
    }
    

    return (HI_VOID *)HI_SUCCESS;
}

HI_S32 Vdec::init(){
    VB_CONFIG_S stVbConfig;
    HI_S32 i, s32Ret = HI_SUCCESS;
    
    SIZE_S stDispSize;
    HI_U32 u32VdecChnNum;
    
    PIC_SIZE_S enDispPicSize;
    // VDEC_ATTR astSampleVdec[VDEC_MAX_CHN_NUM];
    // VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_CHN_NUM];
   
    u32VdecChnNum = 1;
    
	enDispPicSize = P1080P;


    s32Ret =  GetPicSize(enDispPicSize, &stDispSize);
    if(s32Ret != HI_SUCCESS)
    {
        printf("sys get pic size fail for %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 1;
    stVbConfig.astCommPool[0].u32BlkCnt  = 10*u32VdecChnNum;
    stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
                                                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, 0);



    astSampleVdec.enType                           = PT_H264;
    astSampleVdec.u32Width                         = 1920;
    astSampleVdec.u32Height                        = 1080;
    astSampleVdec.enMode                           = VIDEO_MODE_FRAME;
    astSampleVdec.stSapmleVdecVideo.enDecMode      = VIDEO_DEC_MODE_IP;
    astSampleVdec.stSapmleVdecVideo.enBitWidth     = DATA_BITWIDTH_8;
    astSampleVdec.stSapmleVdecVideo.u32RefFrameNum = 2;
    astSampleVdec.u32DisplayFrameNum               = 2;
    astSampleVdec.u32FrameBufCnt = astSampleVdec.stSapmleVdecVideo.u32RefFrameNum + astSampleVdec.u32DisplayFrameNum + 1;

    s32Ret = VDEC_InitVBPool(u32VdecChnNum, &astSampleVdec);
    if(s32Ret != HI_SUCCESS)
    {
        printf("init mod common vb fail for %#x!\n", s32Ret);
        VDEC_ExitVBPool();
        return HI_FAILURE;
    }
    /************************************************
    step3:  start VDEC
    *************************************************/
    s32Ret = VDEC_Start(u32VdecChnNum, &astSampleVdec);
    if(s32Ret != HI_SUCCESS)
    {
        printf("start VDEC fail for %#x!\n", s32Ret);
        VDEC_Stop(u32VdecChnNum);
        return HI_FAILURE;
    }


    VPSS_GRP_ATTR_S stVpssGrpAttr;

    stVpssGrpAttr.u32MaxW = 2688;
    stVpssGrpAttr.u32MaxH = 2160;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.bNrEn   = HI_FALSE;

    VPSS_CHN_ATTR_S VpssChnAttr;

    VpssChnAttr.u32Width                    = stDispSize.u32Width;
    VpssChnAttr.u32Height                   = stDispSize.u32Height;
    VpssChnAttr.enChnMode                   = VPSS_CHN_MODE_USER;
    VpssChnAttr.enCompressMode              = COMPRESS_MODE_NONE;
    VpssChnAttr.enDynamicRange              = DYNAMIC_RANGE_SDR8;
    VpssChnAttr.enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    VpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
    VpssChnAttr.stFrameRate.s32DstFrameRate = -1;
    VpssChnAttr.u32Depth                    = 5;
    VpssChnAttr.bMirror                     = HI_FALSE;
    VpssChnAttr.bFlip                       = HI_FALSE;
    VpssChnAttr.stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    VpssChnAttr.enVideoFormat               = VIDEO_FORMAT_LINEAR;

    

    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr); 
	 if (s32Ret != HI_SUCCESS) {
		printf("HI_MPI_VPSS_CreateGrp error %x \n", s32Ret);
	}
	s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &VpssChnAttr);
	if (s32Ret != HI_SUCCESS) {
		printf("HI_MPI_VPSS_SetChnAttr error %x \n", s32Ret);
	}
	s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != HI_SUCCESS) {
		printf("HI_MPI_VPSS_EnableChn error %x \n", s32Ret);
	}
	s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != HI_SUCCESS) {
		printf("HI_MPI_VPSS_StartGrp error %x \n", s32Ret);
	}



    stSrcChn.enModId   = HI_ID_VDEC;
    stSrcChn.s32DevId  = 0;
    stSrcChn.s32ChnId  = 0;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = VpssChn;

    HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);


}