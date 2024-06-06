project(obs-alirtc)

# just sup win x64
SET(ALIRTC_VERSION "6.10.0")
SET(ALIRTC_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libs/include)
SET(ALIRTC_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/x64/Release)

option(ENABLE_ALIRTC "Enable WebRTC Output support" ON)
if(NOT ENABLE_ALIRTC)
  obs_status(DISABLED "obs-alirtc")
  return()
endif()

add_library(obs-alirtc MODULE)
add_library(OBS::alirtc ALIAS obs-alirtc)
set_property(TARGET obs-alirtc PROPERTY IMPORTED_LOCATION "${ALIRTC_LIBRARY}")
set_target_properties(
      obs-alirtc
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ALIRTC_INCLUDE_DIRS}"
                 VERSION ${ALIRTC_VERSION})

target_sources(obs-alirtc PRIVATE obs-alirtc.cpp)


target_link_directories(obs-alirtc PRIVATE "${ALIRTC_LIBRARY}")
target_include_directories(obs-alirtc PRIVATE "${ALIRTC_INCLUDE_DIRS}")
target_link_libraries(obs-alirtc PRIVATE OBS::libobs AliRTCSdk)

set_target_properties(obs-alirtc PROPERTIES FOLDER "plugins")

setup_plugin_target(obs-alirtc)
