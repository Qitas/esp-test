set(COMPONENT_ADD_INCLUDEDIRS ./include)

register_component()

target_link_libraries(${COMPONENT_TARGET} INTERFACE "-L ${CMAKE_CURRENT_SOURCE_DIR}")

add_prebuilt_library(esp-lib "${CMAKE_CURRENT_SOURCE_DIR}/libhello_world_lib.a"
)

target_link_libraries(${COMPONENT_TARGET} INTERFACE hello_world_lib)