file(GLOB PROTO_FILES "*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
add_library(daydream_proto_lib ${PROTO_SRCS})
