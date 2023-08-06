# add_library(libmad INTERFACE)
# target_sources(libmad INTERFACE
#     ${CMAKE_CURRENT_LIST_DIR}/bit.c
#     ${CMAKE_CURRENT_LIST_DIR}/decoder.c
#     ${CMAKE_CURRENT_LIST_DIR}/fixed.c
#     ${CMAKE_CURRENT_LIST_DIR}/frame.c
#     ${CMAKE_CURRENT_LIST_DIR}/huffman.c
#     ${CMAKE_CURRENT_LIST_DIR}/layer3.c
#     ${CMAKE_CURRENT_LIST_DIR}/layer12.c
#     ${CMAKE_CURRENT_LIST_DIR}/stream.c
#     ${CMAKE_CURRENT_LIST_DIR}/synth.c
#     ${CMAKE_CURRENT_LIST_DIR}/timer.c
#     ${CMAKE_CURRENT_LIST_DIR}/version.c
# )
# # target_include_directories(libmad INTERFACE
# #     libmad
# # )
# target_link_libraries(libmad INTERFACE
#         pico_stdlib
# )

# However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.c")

# Generate the shared library from the sources
add_library(libmad STATIC ${SOURCES})

# Specify that this library itself needs access to the folder "dataStructures/include" and that any
# other target which depends on this also gets similar access - i.e. use 'PUBLIC'
target_include_directories(libmad PUBLIC .)

# Specify that this library depends upon cppCommon
target_link_libraries(libmad pico_stdlib)