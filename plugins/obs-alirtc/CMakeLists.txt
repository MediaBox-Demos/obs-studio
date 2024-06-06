cmake_minimum_required(VERSION 3.22...3.25)


legacy_check()

# just sup win x64
SET(ALIRTC_VERSION "6.10.0")
SET(ALIRTC_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libs/include)
SET(ALIRTC_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/x64/Release)
SET(ALIRTC_LIBRARY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/libs/x64/Debug)

option(ENABLE_ALIRTC "Enable AliRtc Output support" ON)
if(NOT ENABLE_ALIRTC)
  target_disable(obs-alirtc)
  return()
endif()

add_library(obs-alirtc MODULE)
add_library(OBS::alirtc ALIAS obs-alirtc)
# add_library(ALIRTC::ALIRTC INTERFACE IMPORTED)
# set_property(TARGET obs-alirtc PROPERTY IMPORTED_LOCATION "${ALIRTC_LIBRARY}")
set_target_properties(
      obs-alirtc
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ALIRTC_INCLUDE_DIRS}"
                 VERSION ${ALIRTC_VERSION})

target_sources(
  obs-alirtc PRIVATE # cmake-format: sortable
                     obs-alirtc.cpp 
                     alirtc_service.h alirtc_service.cpp 
                     alirtc_output.cpp alirtc_output.h 
                     alirtc_encoder.cpp alirtc_encoder.h
                     alirtc_common.cpp alirtc_common.h
                     )

target_link_directories(obs-alirtc PRIVATE "${ALIRTC_LIBRARY}")
target_include_directories(obs-alirtc PRIVATE "${ALIRTC_INCLUDE_DIRS}")
target_link_libraries(obs-alirtc PRIVATE OBS::libobs AliRTCSdk)


# add_custom_command(
#     TARGET obs-alirtc
#     POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E echo "Copy dependencies to binary directory (${OBS_EXECUTABLE_DESTINATION})..."
#     COMMAND "${CMAKE_COMMAND}" -E make_directory "${OBS_OUTPUT_DIR}/$<CONFIG>/${OBS_EXECUTABLE_DESTINATION}"
#     COMMAND "${CMAKE_COMMAND}" -E "$<IF:$<CONFIG:Debug>,copy_if_different,true>"
#             "$<$<CONFIG:Debug>:${ALIRTC_LIBRARY_DEBUG}>" "${OBS_OUTPUT_DIR}/$<CONFIG>/${OBS_EXECUTABLE_DESTINATION}"
#     COMMAND
#       "${CMAKE_COMMAND}" -E "$<IF:$<CONFIG:RelWithDebInfo>,copy_if_different,true>"
#       "$<$<CONFIG:RelWithDebInfo>:${ALIRTC_LIBRARY}>"
#       "${OBS_OUTPUT_DIR}/$<CONFIG>/${OBS_EXECUTABLE_DESTINATION}"
#     COMMAND "${CMAKE_COMMAND}" -E "$<IF:$<CONFIG:Release>,copy_if_different,true>"
#             "$<$<CONFIG:Release>:${ALIRTC_LIBRARY}>" "${OBS_OUTPUT_DIR}/$<CONFIG>/${OBS_EXECUTABLE_DESTINATION}"
#     COMMAND
#       "${CMAKE_COMMAND}" -E "$<IF:$<CONFIG:MinSizeRel>,copy_if_different,true>"
#       "$<$<CONFIG:MinSizeRel>:${ALIRTC_LIBRARY}>" "${OBS_OUTPUT_DIR}/$<CONFIG>/${OBS_EXECUTABLE_DESTINATION}"
#     COMMENT "."
#     VERBATIM COMMAND_EXPAND_LISTS)

  # install(
  #   FILES ${ALIRTC_LIBRARY_DEBUG}
  #   CONFIGURATIONS Debug
  #   DESTINATION "${OBS_EXECUTABLE_DESTINATION}"
  #   COMPONENT Runtime)

  # install(
  #   FILES ${ALIRTC_LIBRARY}
  #   CONFIGURATIONS RelWithDebInfo
  #   DESTINATION "${OBS_EXECUTABLE_DESTINATION}"
  #   COMPONENT Runtime)

  # install(
  #   FILES ${ALIRTC_LIBRARY}
  #   CONFIGURATIONS Release
  #   DESTINATION "${OBS_EXECUTABLE_DESTINATION}"
  #   COMPONENT Runtime)

  # install(
  #   FILES ${ALIRTC_LIBRARY}
  #   CONFIGURATIONS MinSizeRel
  #   DESTINATION "${OBS_EXECUTABLE_DESTINATION}"
  #   COMPONENT Runtime)
    
# cmake-format: off
set_target_properties_obs(obs-alirtc PROPERTIES FOLDER plugins PREFIX "")
# cmake-format: on
