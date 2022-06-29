set(SKIA_INCLUDE_DIRS ${EXTERNAL_DIR}/skia)
set(SKIA_LIBRARIES skia.dll.lib)
set(SKIA_LIBRARY_DIRS ${EXTERNAL_DIR}/skia/lib)

set(NGFX_TEST_REF_SKIA_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

function(build_test_ref name)
file(GLOB_RECURSE TEST_REF_SOURCE_FILES ${name}/*.cpp ${name}/*.h ${name}/*.mm)
add_executable(test_ref_${name} ${TEST_REF_SOURCE_FILES})
target_include_directories(test_ref_${name} PRIVATE ${SKIA_INCLUDE_DIRS})
target_link_libraries(test_ref_${name} PRIVATE ${SKIA_LIBRARIES})
target_link_directories(test_ref_${name} PRIVATE ${SKIA_LIBRARY_DIRS})
target_compile_definitions(test_ref_${name} PRIVATE -DNGFX_TEST_REF_SKIA_DATA_DIR="${NGFX_TEST_REF_SKIA_DATA_DIR}")
endfunction()

build_test_ref(blend)
