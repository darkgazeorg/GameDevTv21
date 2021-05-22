cmake_minimum_required(VERSION 3.0)

set(PNAME "game_jam_2021")
project(${PNAME})

include(ExternalProject)

if(DEFINED FETCH_GGE)
  if(NOT DEFINED GGE_URL)
    set(GGE_URL "https://repo.darkgaze.org/Gorgon")
  endif()
  set(GGE_DIR ${CMAKE_SOURCE_DIR}/gge)
  set(GGE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/gge_install)
  set(GGE_CMAKE_ARGS
    -D BUILD_DOCUMENTATION=OFF
    -D CMAKE_EXPORT_COMPILE_COMMANDS=YES
    -D CMAKE_BUILD_TYPE=Debug
    -D CMAKE_INSTALL_PREFIX=${GGE_INSTALL_PREFIX}
  )
  ExternalProject_Add(gge
    PREFIX "gge"
    HG_REPOSITORY ${GGE_URL}
    HG_TAG "4.x-dev"
    BUILD_ALWAYS TRUE
    SOURCE_DIR ${GGE_DIR}
    CMAKE_ARGS ${GGE_CMAKE_ARGS}
  )

  set(GAME_CMAKE_ARGS
    -D GGE_IN_SOURCE_BUILD=TRUE
    -D GGE_INSTALL_PREFIX=${GGE_INSTALL_PREFIX}
  )
endif()

set(GAME_DIR ${CMAKE_SOURCE_DIR}/game)
set(GAME_CMAKE_ARGS
  ${GAME_CMAKE_ARGS}
  -D RUNTIME_DIR=${CMAKE_SOURCE_DIR}/bin
  -D PNAME=${PNAME}
)
ExternalProject_Add(game
  PREFIX "game"
  DOWNLOAD_COMMAND ""
  BUILD_ALWAYS TRUE
  SOURCE_DIR ${GAME_DIR}
  CMAKE_ARGS ${GAME_CMAKE_ARGS}
  INSTALL_COMMAND ""
)

if(DEFINED FETCH_GGE)
  ExternalProject_Add_StepDependencies(game build gge)
endif()
