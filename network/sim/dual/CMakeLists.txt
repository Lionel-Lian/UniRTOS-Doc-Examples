message(STATUS "cmake config ${CMAKE_CURRENT_SOURCE_DIR}")

if(DEFINED ENV{UNIRTOS_APP_TARGET_NAME})
    set(target $ENV{UNIRTOS_APP_TARGET_NAME})
else()
    get_filename_component(_parent_dir ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
    get_filename_component(_parent_name ${_parent_dir} NAME)
    get_filename_component(_app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(target "${_parent_name}_${_app_name}")
endif()
message(STATUS "UniRTOS app target name: ${target}")
add_apps_libraries($<TARGET_FILE:${target}>)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_unir_lib_dir})

target_include_directories(${target} PRIVATE
    ${SOURCE_TOP_DIR}
)

target_include_directories(${target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB APP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

target_sources(${target} PRIVATE ${APP_SRC})

set(_app_external_library_targets)
if(DEFINED UNIRTOS_EXTERNAL_LIBRARY_TARGETS AND NOT "${UNIRTOS_EXTERNAL_LIBRARY_TARGETS}" STREQUAL "")
    list(APPEND _app_external_library_targets ${UNIRTOS_EXTERNAL_LIBRARY_TARGETS})
endif()

get_property(_global_external_library_targets GLOBAL PROPERTY unirtos_external_library_targets)
if(_global_external_library_targets)
    list(APPEND _app_external_library_targets ${_global_external_library_targets})
endif()

if(_app_external_library_targets)
    list(REMOVE_DUPLICATES _app_external_library_targets)
    message(STATUS "[app-target-link] ${target} links external library targets from env_config: ${_app_external_library_targets}")
    target_link_libraries(${target} PUBLIC ${_app_external_library_targets})
endif()