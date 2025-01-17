option(WINTER_BUILD_EXAMPLES ON)

add_subdirectory(dep/glad)
include_directories(dep/glad/include)

add_subdirectory(dep/glfw)
include_directories(dep/glfw/include)

add_subdirectory(dep/stb_image)
include_directories(dep/stb_image/include)

add_subdirectory(dep/assimp)
include_directories(dep/assimp/include)

add_subdirectory(dep/freetype)
include_directories(dep/freetype/include)

include_directories(dep/glm/)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

add_executable (WinterMain ${SOURCE_FILES})
set_property(TARGET WinterMain PROPERTY CXX_STANDARD 20)

target_link_libraries(WinterMain 
	PUBLIC
	glad 
	glfw 
	PRIVATE
	stb_image 
	assimp 
	freetype
)

add_library(Winter ${SOURCE_FILES})
set_property(TARGET Winter PROPERTY CXX_STANDARD 20)
target_link_libraries(Winter 
	PRIVATE
	glfw 
	glad
	stb_image 
	assimp 
	freetype
)


if(WINTER_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
