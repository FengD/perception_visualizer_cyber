// Copyright (C) 2023 FengD
// License: Modified BSD Software License Agreement
// Author: 3rdparty
// Description: h264 encoder

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x264.h>
#include <stdbool.h>

namespace crdc {
namespace airi {

/**
 * @class H264EncoderData
 * @brief This struct is the data structure of encoder data of h264
 */
typedef struct{
  int raw_frame_size;
  x264_picture_t pic;
  x264_picture_t pic_out;
  bool pic_valid;
  int i_frame;
  x264_nal_t *nal;
  int i_nal;
  x264_t* h264_encoder;
} H264EncoderData;

/**
 * @brief free the context that encoder_data points with
 * @param [in] the input h264 encode data
 */
void encoder_dispose(H264EncoderData* encoder_data);

/**
 * @brief create a new context and let p_encoder_data point to that
 * @param [in] the input h264 encode data
 * @param [in] the width of the input
 * @param [in] the height of the input
 * @param [in] is loss less
 * @return is the action success = 0 means success
 */
int encoder_init(H264EncoderData** p_encoder_data, int width, int height, bool lossless);

/**
 * @brief like the name
 * @param [out] the output of the encoder raw data
 * @return the raw data in the encoder data
 */
uint8_t* encoder_get_raw_data_buf(H264EncoderData* encoder_data);

/**
 * @brief encode the data
 * @param assume the RGB-raw data already in "encoder_get_raw_data_buf()"
 * @param p_encoded_buf[out]: points to the buffer if frame emitted, NULL for no frame emitted
 * @param p_encoded_size[out]: the size of encoded data, 0 when no frame emitted
 * @return is the action success = 0 means success
 */
int encoder_encode(H264EncoderData* encoder_data , uint8_t** p_encoded_buf, int* p_encoded_size);

}  // namespace airi
}  // namespace crdc
