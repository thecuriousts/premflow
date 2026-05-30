cmake_minimum_required(VERSION 3.14)
project(premflow VERSION 0.2.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

set(ELOMAXZ_SOURCE_DIR "" CACHE PATH "Use local elomaxz clone instead of fetching from GitHub")

if(ELOMAXZ_SOURCE_DIR)
    set(FETCHCONTENT_SOURCE_DIR_ELOMAXZ "${ELOMAXZ_SOURCE_DIR}")
endif()

FetchContent_Declare(
    elomaxz
    GIT_REPOSITORY https://github.com/p10ns11y/elomaxz.git
    GIT_TAG        a03078b
)

FetchContent_GetProperties(elomaxz)
if(NOT elomaxz_POPULATED)
    FetchContent_MakeAvailable(elomaxz)
endif()
if(NOT TARGET elomaxz)
    add_library(elomaxz STATIC ${elomaxz_SOURCE_DIR}/src/elomaxz.c)
    target_include_directories(elomaxz PUBLIC ${elomaxz_SOURCE_DIR}/include)
    target_compile_features(elomaxz PUBLIC c_std_11)
    target_compile_options(elomaxz PRIVATE -Wall -Wextra)
endif()

add_executable(premflow
    main.c
    app.c
    effects.c
    core.c
    ui.c
)
target_link_libraries(premflow PRIVATE elomaxz)
target_compile_features(premflow PUBLIC c_std_11)
target_compile_options(premflow PRIVATE -Wall -Wextra -O2)

add_executable(test_runner test.c core.c)
target_compile_features(test_runner PUBLIC c_std_11)
target_compile_options(test_runner PRIVATE -Wall -Wextra -O2)

enable_testing()
add_test(NAME unit COMMAND test_runner)

install(TARGETS premflow RUNTIME DESTINATION bin)
