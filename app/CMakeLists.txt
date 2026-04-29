add_executable(HoroStarterApp main.cpp)

target_link_libraries(HoroStarterApp PRIVATE HoroEngine)

if(HORO_RENDERER STREQUAL "opengl")
    target_link_libraries(HoroStarterApp PRIVATE horo-renderer-opengl)
elseif(HORO_RENDERER STREQUAL "null")
    target_link_libraries(HoroStarterApp PRIVATE horo-renderer-null)
endif()

add_custom_command(TARGET HoroStarterApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:HoroStarterApp>/assets
    COMMENT "Copying starter assets next to HoroStarterApp"
)

if(APPLE)
    set_target_properties(HoroStarterApp PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
