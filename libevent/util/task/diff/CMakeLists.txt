aux_source_directory("." I_SRC)
aux_source_directory("MTSearch_thrift/gen-cpp/" G_SRC)
set(SRC_LIST ${I_SRC} ${G_SRC})

add_executable(qs-diff ${SRC_LIST})
target_link_libraries(qs-diff pthread thrift)

