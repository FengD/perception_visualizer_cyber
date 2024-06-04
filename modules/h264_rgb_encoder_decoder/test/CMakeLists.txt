project(h264_rgb_encoder_decoder_test)

if (GTEST_FOUND)
    add_executable(${PROJECT_NAME}_encoder_decoder_test encoder_decoder_test.cc)
    add_dependencies(${PROJECT_NAME}_encoder_decoder_test h264_rgb_encoder_decoder)
    target_link_libraries(${PROJECT_NAME}_encoder_decoder_test
        gtest pthread
        x264 h264_rgb_encoder_decoder
        swscale avcodec avutil h264_rgb_encoder_decoder
    )
    add_test(${PROJECT_NAME}_encoder_decoder_test ${PROJECT_NAME}_encoder_decoder_test)

else()
    message(WARNING "Gtest not Found. test_tf_op will not build")
endif()
