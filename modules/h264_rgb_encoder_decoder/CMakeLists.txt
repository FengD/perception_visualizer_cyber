project(h264_rgb_encoder_decoder)

if (WITH_COV)
  message(STATUS "WITH_COV ON")
  add_compile_options(--coverage -fno-inline -fno-inline-small-functions -fno-default-inline -fprofile-arcs -ftest-coverage)
  link_libraries(gcov)
  add_definitions(-DWITH_TEST)
else()
  message(STATUS "WITH_COV OFF")
endif()

if (DO_TEST)
    add_subdirectory(test)
endif()

add_subdirectory(examples)

file(GLOB SRCS *.cc)
file(GLOB HEADERS *.h)
add_library(${PROJECT_NAME} SHARED ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${LIB_VERSION} SOVERSION 1)
target_link_libraries(${PROJECT_NAME} x264 swscale avcodec avutil )
install(FILES ${HEADERS} DESTINATION include/h264_rgb_encoder_decoder/)
install(TARGETS ${PROJECT_NAME} DESTINATION lib/)