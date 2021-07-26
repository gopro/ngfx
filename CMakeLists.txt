cmake_minimum_required(VERSION 3.15)
if(MSVC)
# Enable MSVC Runtime Library Property
cmake_policy(SET CMP0091 NEW)
endif()

project(ngfx)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external CACHE STRING "External Directory")
set(CURRENT_VERSION 1.0.0)

option(NGFX_GRAPHICS_BACKEND_METAL "build ngfx metal backend" OFF)
option(NGFX_GRAPHICS_BACKEND_DIRECT3D12 "build ngfx directx12 backend" OFF)
option(NGFX_GRAPHICS_BACKEND_VULKAN "build ngfx vulkan backend" OFF)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library")
endif()

set(CMAKE_EXECUTE_PROCESS_COMMAND_ECHO STDOUT) # echo all process commands

include(GNUInstallDirs)

set(NGFX_DATA_DIR ${CMAKE_INSTALL_FULL_DATADIR}/ngfx/data)

find_package(PkgConfig REQUIRED)

set(STB_INCLUDE_DIRS ${EXTERNAL_DIR}/stb)
set(JSON_DIR ${EXTERNAL_DIR}/json)
set(JSON_INCLUDE_DIRS ${JSON_DIR}/include)

pkg_check_modules(SHADERC REQUIRED shaderc)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(SPIRV_CROSS_DIR ${EXTERNAL_DIR}/windows)
  set(GLM_INCLUDE_DIRS ${EXTERNAL_DIR}/glm-0.9.9.8/glm)

  if(NGFX_GRAPHICS_BACKEND_DIRECT3D12)
    set(D3DX12_INCLUDE_DIRS ${EXTERNAL_DIR}/d3dx12)
  elseif(NGFX_GRAPHICS_BACKEND_VULKAN)
    set(VULKANSDK_DIR "Vulkan SDK directory" C:/VulkanSDK/1.2.131.2)
    set(VULKANSDK_INCLUDE_DIRS ${VULKANSDK_DIR}/Include)
    set(VULKANSDK_LIBRARY_DIRS ${VULKANSDK_DIR}/Lib)
    set(VULKANSDK_LIBRARIES vulkan-1)
  endif()

  set(GLFW_DIR ${EXTERNAL_DIR}/glfw-3.3.2.bin.WIN64)
  set(GLFW_INCLUDE_DIRS ${GLFW_DIR}/include)
  set(GLFW_LIBRARY_DIRS ${GLFW_DIR}/lib-vc2019)
  set(GLFW_LIBRARIES glfw3)
else()
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      set(SPIRV_CROSS_DIR /usr/local)
      pkg_check_modules(GLM REQUIRED glm)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(SPIRV_CROSS_DIR /usr/local/opt/spirv-cross)
      set(GLM_INCLUDE_DIRS /usr/local/include)
  endif()
endif()

set(SPIRV_CROSS_INCLUDE_DIRS ${SPIRV_CROSS_DIR}/include)
set(SPIRV_CROSS_LIBRARY_DIRS ${SPIRV_CROSS_DIR}/lib)
set(SPIRV_CROSS_LIBRARIES spirv-cross-cpp spirv-cross-core spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util spirv-cross-c)

file(GLOB_RECURSE NGFX_SOURCE_FILES
  src/ngfx/core/* src/ngfx/compute/*
  src/ngfx/computeOps/* src/ngfx/drawOps/* src/ngfx/graphics/* src/ngfx/input/*
  src/ngfx/regex/*
)
if(NGFX_GRAPHICS_BACKEND_VULKAN)
    set(WINDOW_BACKEND_GLFW 1)
    file(GLOB_RECURSE NGFX_GRAPHICS_BACKEND_SOURCE_FILES src/ngfx/porting/vulkan/*.cpp src/ngfx/porting/vulkan/*.h)
    set(NGFX_GRAPHICS_BACKEND_CFLAGS -DNGFX_GRAPHICS_BACKEND_VULKAN)
    set(NGFX_GRAPHICS_BACKEND_LIBS vulkan)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(WINDOW_BACKEND_CFLAGS -DVK_USE_PLATFORM_XCB_KHR -DVK_USE_PLATFORM_XLIB_KHR)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(WINDOW_BACKEND_CFLAGS -DVK_USE_PLATFORM_WIN32_KHR)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(WINDOW_BACKEND_CFLAGS -DVK_USE_PLATFORM_METAL_EXT)
    endif()
elseif(NGFX_GRAPHICS_BACKEND_DIRECT3D12)
    set(WINDOW_BACKEND_WINDOWS 1)
    file(GLOB_RECURSE NGFX_GRAPHICS_BACKEND_SOURCE_FILES src/ngfx/porting/d3d/*.cpp src/ngfx/porting/d3d/*.h)
    set(NGFX_GRAPHICS_BACKEND_CFLAGS -DNGFX_GRAPHICS_BACKEND_DIRECT3D12 -DUNICODE)
    set(NGFX_GRAPHICS_BACKEND_LIBS d3d12 dxgi d3dcompiler)
elseif(NGFX_GRAPHICS_BACKEND_METAL)
    set(WINDOW_BACKEND_APPKIT 1)
    file(GLOB_RECURSE NGFX_GRAPHICS_BACKEND_SOURCE_FILES src/ngfx/porting/metal/*.cpp src/ngfx/porting/metal/*.h src/ngfx/porting/metal/*.mm)
    set(NGFX_GRAPHICS_BACKEND_CFLAGS -DNGFX_GRAPHICS_BACKEND_METAL)
    set(NGFX_GRAPHICS_BACKEND_LIBS "-framework AppKit -framework MetalKit -framework Metal")
else()
    message(FATAL_ERROR "ERROR: graphics backend not set") 
endif()
if(WINDOW_BACKEND_GLFW)
    file(GLOB_RECURSE WINDOW_BACKEND_SOURCE_FILES src/ngfx/porting/glfw/*.cpp src/ngfx/porting/glfw/*.h)
    set(WINDOW_BACKEND_LIBS glfw)
    set(WINDOW_BACKEND_CFLAGS ${WINDOW_BACKEND_CFLAGS} -DNGFX_WINDOW_BACKEND_GLFW)
elseif(WINDOW_BACKEND_WINDOWS)
    file(GLOB_RECURSE WINDOW_BACKEND_SOURCE_FILES src/ngfx/porting/windows/*.cpp src/ngfx/porting/windows/*.h)
    set(WINDOW_BACKEND_CFLAGS ${WINDOW_BACKEND_CFLAGS} -DNGFX_WINDOW_BACKEND_WINDOWS)
elseif(WINDOW_BACKEND_APPKIT)
    file(GLOB_RECURSE APP_BACKEND_SOURCE_FILES src/ngfx/porting/appkit/*)
    set(APP_BACKEND_SOURCE_FILES ${APP_BACKEND_SOURCE_FILES} PARENT_SCOPE)
    set(RESOURCE_FILES src/ngfx/porting/appkit/Main.storyboard)
    set(WINDOW_BACKEND_CFLAGS ${WINDOW_BACKEND_CFLAGS} -DNGFX_WINDOW_BACKEND_APPKIT)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${NGFX_SOURCE_FILES} ${NGFX_GRAPHICS_BACKEND_SOURCE_FILES} ${WINDOW_BACKEND_SOURCE_FILES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(NGFX_BUILD_TYPE STATIC)
else()
    set(NGFX_BUILD_TYPE SHARED)
endif()
add_library(ngfx ${NGFX_BUILD_TYPE} ${NGFX_SOURCE_FILES} ${NGFX_GRAPHICS_BACKEND_SOURCE_FILES} ${WINDOW_BACKEND_SOURCE_FILES})

target_compile_definitions(ngfx PUBLIC -DGLM_ENABLE_EXPERIMENTAL -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 
    -DNGFX_DATA_DIR="${NGFX_DATA_DIR}" 
    ${NGFX_GRAPHICS_BACKEND_CFLAGS}
    ${WINDOW_BACKEND_CFLAGS}
)
target_link_libraries(ngfx
    ${NGFX_GRAPHICS_BACKEND_LIBS}
    ${WINDOW_BACKEND_LIBS}
    ${SHADERC_LIBRARIES}
    ${SPIRV_CROSS_LIBRARIES}
)
target_include_directories(ngfx PUBLIC . src
    ${GLM_INCLUDE_DIRS}
    ${STB_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
    ${SHADERC_INCLUDE_DIRS}
    ${SPIRV_CROSS_INCLUDE_DIRS}
)
target_link_directories(ngfx PUBLIC
    ${SHADERC_LIBRARY_DIRS}
    ${SPIRV_CROSS_LIBRARY_DIRS}
)

if(NGFX_GRAPHICS_BACKEND_DIRECT3D12)
    target_include_directories(ngfx PUBLIC ${D3DX12_INCLUDE_DIRS})
elseif(NGFX_GRAPHICS_BACKEND_VULKAN)
    target_include_directories(ngfx PUBLIC ${VULKANSDK_INCLUDE_DIRS})
endif()

if(WINDOW_BACKEND_GLFW)
    target_include_directories(ngfx PUBLIC ${GLFW_INCLUDE_DIRS})
endif()

function(build_test name)
file(GLOB_RECURSE TEST_SOURCE_FILES test/${name}/*.cpp test/${name}/*.h test/${name}/*.mm)
if(NGFX_GRAPHICS_BACKEND_METAL)
add_executable(${name} ${TEST_SOURCE_FILES} ${APP_BACKEND_SOURCE_FILES} ${NGFX_DIR}/src/ngfx/porting/appkit/Main.storyboard)
set(RESOURCE_FILES ${NGFX_DIR}/src/ngfx/porting/appkit/Info.plist ${NGFX_DIR}/src/ngfx/porting/appkit/Main.storyboard)
set_target_properties(${name} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_FRAMEWORK_IDENTIFIER org.gopro.ngfx
    RESOURCE "${RESOURCE_FILES}"
    MACOSX_BUNDLE_INFO_PLIST ${NGFX_DIR}/src/ngfx/porting/appkit/Info.plist
)
else()
add_executable(${name} ${TEST_SOURCE_FILES})
endif()
target_link_libraries(${name} ngfx)
endfunction()

function(build_tool name)
file(GLOB_RECURSE TOOL_SOURCE_FILES tools/${name}.cpp tools/${name}*.h)
add_executable(ngfx_${name} ${TOOL_SOURCE_FILES})
target_link_libraries(ngfx_${name} ngfx ${NGFX_GRAPHICS_BACKEND_LIBS} ${WINDOW_BACKEND_LIBS})
install(TARGETS ngfx_${name}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
endfunction()

message(STATUS "\n"
    "ngfx graphics_backend_cflags: ${NGFX_GRAPHICS_BACKEND_CFLAGS}\n"
    "ngfx window_backend_cflags  : ${WINDOW_BACKEND_CFLAGS}\n"
)

if (NGFX_GRAPHICS_BACKEND_VULKAN)
#build_tool(compile_shaders_vk)
elseif(NGFX_GRAPHICS_BACKEND_DIRECT3D12)
build_tool(compile_shaders_dx12)
elseif(NGFX_GRAPHICS_BACKEND_METAL)
#build_tool(compile_shaders_mtl)
endif()

function(write_pkg_config_file target)

get_target_property(compiler_definitions ${target} COMPILE_DEFINITIONS)
get_target_property(include_directories ${target} INCLUDE_DIRECTORIES)
get_target_property(link_libraries ${target} LINK_LIBRARIES)
get_target_property(link_directories ${target} LINK_DIRECTORIES)

set(CFLAGS "")
foreach(def ${compiler_definitions})
set(CFLAGS "${CFLAGS} -D${def}")
endforeach()
string(REPLACE "-DUNICODE" "" CFLAGS ${CFLAGS})
foreach(dir ${include_directories})
file(TO_CMAKE_PATH  ${dir} dir)
set(CFLAGS "${CFLAGS} -I${dir}")
endforeach()

set(LIBS "-l${target}")
foreach(lib ${link_libraries})
if(lib MATCHES "^-.*")
set(LIBS "${LIBS} ${lib}")
else()
set(LIBS "${LIBS} -l${lib}")
endif()
endforeach()

set(LIB_DIRS "")
foreach(dir ${link_directories})
file(TO_CMAKE_PATH  ${dir} dir)
set(LIB_DIRS "${LIB_DIRS} -L${dir}")
endforeach()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${target}.pc.in ${CMAKE_CURRENT_BINARY_DIR}/${target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

endfunction()

write_pkg_config_file(ngfx)

install(TARGETS ngfx
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

file(GLOB_RECURSE NGFX_DATA_FILES data/*)
install(FILES ${NGFX_DATA_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/ngfx/data)

function(install_headers folder)
file(GLOB_RECURSE NGFX_HEADER_FILES src/${folder}/*.h)
install(FILES ${NGFX_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${folder})
endfunction()

install_headers(ngfx/core)
install_headers(ngfx/compute)
install_headers(ngfx/computeOps)
install_headers(ngfx/drawOps)
install_headers(ngfx/graphics)
install_headers(ngfx/input)
install_headers(ngfx/regex)
install_headers(ngfx/porting/appkit)
install_headers(ngfx/porting/d3d)
install_headers(ngfx/porting/glfw)
install_headers(ngfx/porting/metal)
install_headers(ngfx/porting/vulkan)
install_headers(ngfx/porting/windows)
