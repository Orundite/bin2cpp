cmake_minimum_required(VERSION 3.30)

project(bin2cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bin2cpp bin2cpp.cpp
)
if(WIN32)
    if(MSVC)
        target_compile_options(bin2cpp PRIVATE /utf-8)
    elseif(MINGW)
        target_compile_options(bin2cpp PRIVATE -municode)
        target_link_options(bin2cpp PRIVATE -municode -mconsole)
    endif()
endif()

add_custom_command(
    TARGET bin2cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
            "$<TARGET_FILE_DIR:bin2cpp>/LICENSE"
    COMMENT "Copying LICENSE to the build directory..."
    VERBATIM
)

include(GNUInstallDirs)
install(TARGETS bin2cpp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES LICENSE 
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)