#include <gtest/gtest.h>
#include "h264_rgb_encoder_decoder/encoder.h"
#include "h264_rgb_encoder_decoder/decoder.h"

namespace crdc {
namespace airi {

class EncoderDecoderTest : public ::testing::Test {
protected:
    H264EncoderData* encoder_data;
    H264DecoderData* decoder_data;

    void SetUp() override {
        ASSERT_EQ(encoder_init(&encoder_data, 640, 480, false), 0);
        ASSERT_EQ(decoder_init(&decoder_data), 0);
    }

    void TearDown() override {
        encoder_dispose(encoder_data);
        decoder_dispose(decoder_data);
    }
};

TEST_F(EncoderDecoderTest, EncoderDecoderInitialization) {
    EXPECT_NE(encoder_data, nullptr);
    EXPECT_NE(decoder_data, nullptr);
}

TEST_F(EncoderDecoderTest, EncoderFunctionality) {
    uint8_t* raw_data_buf = encoder_get_raw_data_buf(encoder_data);
    ASSERT_NE(raw_data_buf, nullptr);

    uint8_t* encoded_buf;
    int encoded_size;

    encoder_encode(encoder_data, &encoded_buf, &encoded_size);

    EXPECT_NE(encoded_buf, nullptr);
    EXPECT_GT(encoded_size, 0);
}

TEST_F(EncoderDecoderTest, DecoderFunctionality) {
    uint8_t* raw_data_buf = encoder_get_raw_data_buf(encoder_data);
    ASSERT_NE(raw_data_buf, nullptr);

    uint8_t* encoded_buf;
    int encoded_size;

    encoder_encode(encoder_data, &encoded_buf, &encoded_size);
    ASSERT_NE(encoded_buf, nullptr);
    ASSERT_GT(encoded_size, 0);

    int frame_formed = decoder_parse(decoder_data, encoded_buf, encoded_size);
    EXPECT_GE(frame_formed, 0);
}
}  // namespace airi
}  // namespace crdc

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
