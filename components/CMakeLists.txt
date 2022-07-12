set(COMPONENT_ADD_INCLUDEDIRS ./include)

register_component()

target_link_libraries(${COMPONENT_TARGET} INTERFACE "-L ${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_prebuilt_library(hello-world "${CMAKE_CURRENT_SOURCE_DIR}/lib/libhello_world.a")

target_link_libraries(${COMPONENT_TARGET} INTERFACE hello_world)