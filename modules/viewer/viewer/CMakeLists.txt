add_subdirectory(proto)
file(GLOB_RECURSE SRCS "*.cc")
add_executable(viewer ${SRCS})
target_link_libraries(viewer ${VIEWER_DEPENDS})

install(TARGETS viewer RUNTIME DESTINATION ./viewer/bin)
install(DIRECTORY params DESTINATION ./viewer/)
