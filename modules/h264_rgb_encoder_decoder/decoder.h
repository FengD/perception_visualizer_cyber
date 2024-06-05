// Copyright (C) 2023 FengD
// License: Modified BSD Software License Agreement
// Author: 3rdparty
// Description: h264 decoder

#pragma once

#include <stdio.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace crdc {
namespace airi {

struct _H264DecoderData;

/**
 * @brief the decoder action callback
 */
typedef void (*handler_on_frame_ready)(struct _H264DecoderData* decoder_data, uint8_t* frame_buf, int frame_size);

/**
 * @class H264DecoderData
 * @brief This struct is the data structure of decoder data of h264
 */
struct _H264DecoderData {
  struct AVCodec *pCodec;
  struct AVCodecContext *pCodecCtx;
  struct AVCodecParserContext *pCodecParserCtx;
  struct AVFrame *pFrame,*pFrameOutput;
  uint8_t *out_buffer;
  struct AVPacket packet;
  enum AVCodecID codec_id;
  struct SwsContext *img_convert_ctx;
  int first_time;
  int width;
  int height;
  int output_size;
  void* user_data;
  handler_on_frame_ready frame_handler;
};

typedef struct _H264DecoderData H264DecoderData;

/**
 * @brief dispose the decoder action
 * @param [in] the input h264 data
 * @return is the action success = 0 means success
 */
int decoder_init(H264DecoderData** p_decoder_data);

/**
 * @brief dispose the decoder data, used in deinit action
 * @param [in] the input h264 decode data
 */
void decoder_dispose(H264DecoderData* decoder_data);

/**
 * @brief set a custom handler action for decoder callback
 * @param [in] the input h264 decode data
 * @param [in] the callback fn
 */
handler_on_frame_ready decoder_set_frame_ready_handler(H264DecoderData* decoder_data, handler_on_frame_ready hnd);

/**
 * @brief do the parser and put the rgb data in decoder_data output data
 * @param [in] input h264 decode data
 * @param [in] the input h264 data
 * @param [in] the input data size
 * @return is the action success = 0 means success
 */
int decoder_parse(H264DecoderData* decoder_data,uint8_t* in_buffer, int cur_size);

/**
 * @brief flush the h264 decode data used in deinit step
 * @param [in] input h264 decode data
 * @return is the action success = 0 means success
 */
int decoder_flush(H264DecoderData* decoder_data);

/**
 * @brief get the padding of the h264 data header size if need
 * @return the size of padding
 */
int decoder_input_buffer_padding_size();

}  // namespace airi
}  // namespace crdc
