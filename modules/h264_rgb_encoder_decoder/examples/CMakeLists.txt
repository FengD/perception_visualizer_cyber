project(h264_rgb_encoder_decoder_examples)

add_executable(${PROJECT_NAME}_decoder decoder_example.cc)
add_dependencies(${PROJECT_NAME}_decoder h264_rgb_encoder_decoder)
target_link_libraries(${PROJECT_NAME}_decoder swscale avcodec avutil h264_rgb_encoder_decoder)

add_executable(${PROJECT_NAME}_encoder encoder_example.cc)
add_dependencies(${PROJECT_NAME}_encoder h264_rgb_encoder_decoder)
target_link_libraries(${PROJECT_NAME}_encoder x264 h264_rgb_encoder_decoder)

install(TARGETS ${PROJECT_NAME}_decoder DESTINATION bin/h264_rgb_encoder_decoder/)
install(TARGETS ${PROJECT_NAME}_encoder DESTINATION bin/h264_rgb_encoder_decoder/)
