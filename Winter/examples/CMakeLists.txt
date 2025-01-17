function(create_project_from_sources example)
	file(GLOB SOURCE ${example}/*.h ${example}/*.cpp)
    add_executable(${example} ${SOURCE})
    include_directories(${CMAKE_SOURCE_DIR}/Winter/src)
    target_link_libraries(${example} Winter)
    set_property(TARGET ${example} PROPERTY CXX_STANDARD 20)
endfunction()

file(GLOB EXAMPLES *)
foreach(EXAMPLE ${EXAMPLES})
    if(IS_DIRECTORY ${EXAMPLE})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME)
        message("Creating example: "${EXAMPLE_NAME})
        create_project_from_sources(${EXAMPLE_NAME})
    endif()
endforeach()


