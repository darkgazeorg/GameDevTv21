cmake_minimum_required(VERSION 3.0)

project(${PNAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(DEFINED GGE_IN_SOURCE_BUILD)
  find_package(Gorgon 4.0.0 REQUIRED PATHS ${GGE_INSTALL_PREFIX} NO_DEFAULT_PATH)
else()
  find_package(Gorgon 4.0.0 REQUIRED)
endif()

set(SOURCES
  src/main.cpp
  src/GameScene.h
  src/Scenes.h
  src/Types.h
  src/Map.h
  src/Map.cpp
  src/MapGen.h
  src/MapGen.cpp
  src/Tower.h
  src/Tower.cpp
  src/ImProc.h
  src/Improc.cpp
  src/Resources.h
  src/Resources.cpp
  src/Enemy.h
  src/Enemy.cpp
)

add_executable(${PNAME} ${SOURCES})
add_custom_command(TARGET ${PNAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PNAME}> ${RUNTIME_DIR})

target_compile_features(${PNAME} PRIVATE cxx_std_14)
linkgorgon(${PNAME})

if(MSVC)
  set(COMPILER_OPTIONS
    /Zi
    /permissive-
    /W4
  )
else()
  set(COMPILER_OPTIONS
    -Wall
    -Wno-unknown-pragmas
    -Wno-comment
    -Wno-reorder
    -Wno-unused-local-typedefs
    -Wno-sign-compare #should not be here
    #-Werror
    #-pedantic-errors
    #-O2
    -ggdb
    -g3
    #-fsanitize=address
  )
  set(LIBRARIES
    #asan
  )
endif()

target_compile_options(${PNAME} PRIVATE ${COMPILER_OPTIONS})

target_link_libraries(${PNAME} ${LIBRARIES})
