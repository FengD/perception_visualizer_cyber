# H264 RGB Encoder Decoder

## Description
A simple C program that convert the raw RGB stream to H264 stream and vice versa.

## For more details

```
# For the use of decode action

// create decoder data
H264DecoderData* decoder_data;

// init action
static inline void init_h264_decoder() {
  if(decoder_init(&decoder_data) < 0){
    printf("Fail to init decoder\n");
  }
}

// deinit action
static inline void deinit_h264_decoder() {
  decoder_parse(decoder_data, NULL, 0);
  decoder_flush(decoder_data);
  decoder_dispose(decoder_data);
}

// parse action
decoder_parse(decoder_data, const_cast<unsigned char*>(ros2_image.data.data()), ros2_image.data.size());

// height
decoder_data->height

// width
decoder_data->width

// the rgb result
decoder_data->out_buffer
``