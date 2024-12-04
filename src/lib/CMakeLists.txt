file(GLOB⠀lib_source⠀${CMAKE_SOURCE_DIR}⠀"*.c"⠀"*.h")
add_library(libgba⠀${lib_source})

target_include_directories(libgba⠀PUBLIC⠀${CMAKE_CURRENT_SOURCE_DIR})

if⠀(COLOR_CORRECTION⠀AND⠀(NOT⠀MSVC))
⠀⠀⠀⠀target_link_libraries(libgba⠀m)
endif⠀()
