cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_SDL3 main.cpp ../../.clang-format)
target_link_libraries(sample_SDL3 PRIVATE input::SDL3 input::test SDL3_ttf::SDL3_ttf)

set(DEPENDENCY_FILES $<TARGET_FILE:SDL3::SDL3>)

if(TARGET SDL3_ttf::SDL3_ttf)
    target_link_libraries(sample_SDL3 PRIVATE SDL3_ttf::SDL3_ttf)
    list(APPEND DEPENDENCY_FILES $<TARGET_FILE:SDL3_ttf::SDL3_ttf> $<TARGET_FILE:Freetype::Freetype> $<TARGET_FILE:harfbuzz> $<TARGET_FILE:plutosvg::plutosvg> $<TARGET_FILE:plutovg::plutovg> )
elseif(TARGET SDL3_ttf::SDL3_ttf-static)
    target_link_libraries(sample_SDL3 PRIVATE SDL3_ttf::SDL3_ttf-static)
endif()

# Copy File dependencies to the output folder.
add_custom_command(TARGET sample_SDL3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEPENDENCY_FILES}
    $<TARGET_FILE_DIR:sample_SDL3>
)
