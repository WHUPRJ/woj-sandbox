cmake_minimum_required(VERSION 3.23)
project(woj_sandbox C)

set(CMAKE_C_STANDARD 23)

# Source files
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/rules/*.c ${PROJECT_SOURCE_DIR}/utils/*.c)
list(FILTER SRC_FILES EXCLUDE REGEX ".*test\\.c$")
set(VERSION_SCRIPT ${PROJECT_SOURCE_DIR}/version_script.txt)

# Include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libseccomp/include)

# Targets
add_library(woj_sandbox SHARED ${SRC_FILES})
add_executable(woj_sandbox_test ${PROJECT_SOURCE_DIR}/test.c ${SRC_FILES})

# Link seccomp
target_link_libraries(woj_sandbox ${CMAKE_SOURCE_DIR}/libseccomp/src/.libs/libseccomp.a)
target_link_libraries(woj_sandbox_test ${CMAKE_SOURCE_DIR}/libseccomp/src/.libs/libseccomp.a)

# Disable symbol export
set_target_properties(woj_sandbox PROPERTIES C_VISIBILITY_PRESET hidden)
set_property(TARGET woj_sandbox APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--version-script=${VERSION_SCRIPT}")
set_target_properties(woj_sandbox PROPERTIES LINK_DEPENDS ${VERSION_SCRIPT})

# Optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_result OUTPUT ipo_output)
if (ipo_result)
    set_property(TARGET woj_sandbox PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else ()
    message(WARNING "IPO is not supported: ${ipo_output}")
endif ()
