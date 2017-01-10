#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "includes/libavcodec/avcodec.h"  
#ifdef __cplusplus
}
#endif
//test different codec  
#define TEST_H264  1  
#define TEST_HEVC  0  

typedef struct sDec {
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx;
    AVCodecParserContext *pCodecParserCtx;  
    AVFrame *pFrame;
    AVPacket packet;   
    char *inputpath;
    char *outputpath;
    enum AVCodecID codecid;
} ffDecoder, *pffDecoder;

int InitDecoder (pffDecoder pDecoderTest) {
#if TEST_HEVC  
    pDecoderTest->codecid = AV_CODEC_ID_HEVC;   
#elif TEST_H264  
    pDecoderTest->codecid = AV_CODEC_ID_H264;  
#else  
    pDecoderTest->codecid = AV_CODEC_ID_MPEG2VIDEO;   
#endif 
    avcodec_register_all();  
  
    pDecoderTest->pCodec = avcodec_find_decoder (pDecoderTest->codecid);  
    if (!pDecoderTest->pCodec) {  
        printf("Codec not found\n");  
        return -1;  
    }
    pDecoderTest->pCodecCtx = avcodec_alloc_context3 (pDecoderTest->pCodec);  
    if (!pDecoderTest->pCodecCtx){  
        printf("Could not allocate video codec context\n");  
        return -1;  
    }
    pDecoderTest->pCodecParserCtx = av_parser_init (pDecoderTest->codecid);  
    if (!pDecoderTest->pCodecParserCtx){  
        printf("Could not allocate video parser context\n");  
        return -1;  
    }
    //if(pCodec->capabilities&CODEC_CAP_TRUNCATED)  
    //    pCodecCtx->flags|= CODEC_FLAG_TRUNCATED;   
      
    if (avcodec_open2 (pDecoderTest->pCodecCtx, pDecoderTest->pCodec, NULL) < 0) {  
        printf("Could not open codec\n");  
        return -1;  
    }

    pDecoderTest->pFrame = av_frame_alloc();
    av_init_packet(&(pDecoderTest->packet));
    return 0;
}

int DecoderDecode (pffDecoder pDecoderTest) {
    AVCodecContext *pCodecCtx = pDecoderTest->pCodecCtx;
    AVCodecParserContext *pCodecParserCtx = pDecoderTest->pCodecParserCtx;
    AVFrame *pFrame = pDecoderTest->pFrame;
    AVPacket packet = pDecoderTest->packet;

    const int in_buffer_size=1024;  
    uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE]={0};  
    uint8_t *cur_ptr;  
    int cur_size;   
    int ret, got_picture;  
    int y_size;

    int first_time=1;  
    //Input File  
    FILE *fp_in = fopen(pDecoderTest->inputpath, "rb");  
    if (!fp_in) {  
        printf("Could not open input stream\n");  
        return -1;  
    }  
    //Output File  
    FILE *fp_out = fopen(pDecoderTest->outputpath, "wb");  
    if (!fp_out) {  
        printf("Could not open output YUV file\n");  
        return -1;  
    }   
    //av_log_set_level(AV_LOG_DEBUG); 
    while (1) {
  
        cur_size = fread (in_buffer, 1, in_buffer_size, fp_in);  
        if (cur_size == 0)  
            break;  
        cur_ptr=in_buffer;  
  
        while (cur_size>0) {  
  
            int len = av_parser_parse2(  
                pCodecParserCtx, pCodecCtx,  
                &packet.data, &packet.size,  
                cur_ptr , cur_size ,  
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
            
            cur_ptr += len;
            cur_size -= len;
            if(packet.size==0)  
                continue;
  
            //Some Info from AVCodecParserContext  
            printf("[Packet]Size:%6d\t",packet.size);  
            switch(pCodecParserCtx->pict_type){  
                case AV_PICTURE_TYPE_I: printf("Type:I\t");break;  
                case AV_PICTURE_TYPE_P: printf("Type:P\t");break;  
                case AV_PICTURE_TYPE_B: printf("Type:B\t");break;  
                default: printf("Type:Other\t");break;  
            }  
            printf("Number:%4d\n",pCodecParserCtx->output_picture_number);  
  
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);  
            if (ret < 0) {  
                printf("Decode Error.\n");  
                return ret;  
            }  
            if (got_picture) {  
                if(first_time){  
                    printf("\nCodec Full Name:%s\n",pCodecCtx->codec->long_name);  
                    printf("width:%d\nheight:%d\n\n",pCodecCtx->width,pCodecCtx->height);  
                    first_time=0;  
                }  
                //Y, U, V  
                for(int i=0;i<pFrame->height;i++){  
                    fwrite(pFrame->data[0]+pFrame->linesize[0]*i,1,pFrame->width,fp_out);  
                }  
                for(int i=0;i<pFrame->height/2;i++){  
                    fwrite(pFrame->data[1]+pFrame->linesize[1]*i,1,pFrame->width/2,fp_out);  
                }  
                for(int i=0;i<pFrame->height/2;i++){  
                    fwrite(pFrame->data[2]+pFrame->linesize[2]*i,1,pFrame->width/2,fp_out);  
                }  
  
                printf("Succeed to decode 1 frame!\n");  
            }  
        }  
  
    }  
  
    //Flush Decoder  
    packet.data = NULL;  
    packet.size = 0;  
    while(1) {  
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);  
        if (ret < 0) {  
            printf("Decode Error.\n");  
            return ret;  
        }  
        if (!got_picture){  
            break;  
        }else {  
            //Y, U, V  
            for(int i=0;i<pFrame->height;i++){  
                fwrite(pFrame->data[0]+pFrame->linesize[0]*i,1,pFrame->width,fp_out);  
            }  
            for(int i=0;i<pFrame->height/2;i++){  
                fwrite(pFrame->data[1]+pFrame->linesize[1]*i,1,pFrame->width/2,fp_out);  
            }  
            for(int i=0;i<pFrame->height/2;i++){  
                fwrite(pFrame->data[2]+pFrame->linesize[2]*i,1,pFrame->width/2,fp_out);  
            }  
  
            printf("Flush Decoder: Succeed to decode 1 frame!\n");  
        }  
    }  
  
    fclose(fp_in);
    fclose(fp_out);
}

int DeInitDecoder (pffDecoder pDecoderTest) {

    av_parser_close(pDecoderTest->pCodecParserCtx);
  
    av_frame_free(&(pDecoderTest->pFrame));
    avcodec_close(pDecoderTest->pCodecCtx);
    av_free(pDecoderTest->pCodecCtx);

    return 0; 
}

int main (int argc, char* argv[]) {
    ffDecoder fDecoderTest;
    memset (&fDecoderTest, 0, sizeof(ffDecoder));
    pffDecoder ffDecoderTest = &fDecoderTest;

    ffDecoderTest->inputpath = argv[1];
    ffDecoderTest->outputpath = argv[2];
    if (argc != 3)
        printf("input: %s instream outstream\n", argv[0]);
    InitDecoder (ffDecoderTest);

    DecoderDecode (ffDecoderTest);

    DeInitDecoder (ffDecoderTest);

    return 0;  
} 