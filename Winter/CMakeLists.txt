cmake_minimum_required (VERSION 3.12)

set(BUILD_EXAMPLES true)


add_subdirectory(dep/glad)
include_directories(dep/glad/include)

add_subdirectory(dep/glfw-3.4)
include_directories(dep/glfw-3.4/include)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
add_executable (Winter ${SOURCE_FILES})
set_property(TARGET Winter PROPERTY CXX_STANDARD 20)
target_link_libraries(Winter glad glfw)

if(${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()


