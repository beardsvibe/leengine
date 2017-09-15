
# to compile:

# - emscripten, run cmake -DCMAKE_COMPILER_IS_MINGW=1 -DCMAKE_TOOLCHAIN_FILE=%EMSCRIPTEN%/cmake/Modules/Platform/Emscripten.cmake -GNinja ..
# - android, use android studio
# - iOS, first run cmake -DIOS=1 -GNinja .. to generate all temporary files, then use XCode
# - macOS, run cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 -GNinja ..
# - win, use plain cmake

cmake_minimum_required(VERSION 3.3)

option(BGFX_DEBUG		"BGFX debug libraries" OFF)
option(FMOD_DEBUG		"FMOD debug and live update" OFF)
option(FMOD_DISABLE		"disable FMOD and all sounds" OFF)
option(NO_ATLAS			"disable atlases" OFF)
option(NO_BATCHING		"disable batching" OFF)

# ----------------------------------------------------------------------------------
# project core config

set(PRJ_TARGET "leengine")
set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})
project(${PRJ_TARGET})

# ----------------------------------------------------------------------------------
# cleanup global compiler flag to get more refined control

if(WIN32)
	# replace /MD -> /MT
	set(compiler_flags
		CMAKE_C_FLAGS
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_RELWITHDEBINFO
		CMAKE_C_FLAGS_MINSIZEREL
		CMAKE_CXX_FLAGS
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS_MINSIZEREL
	)
	foreach(compiler_flag ${compiler_flags})
		string(REPLACE "/MD" "/MT" ${compiler_flag} "${${compiler_flag}}")
	endforeach()
endif()

separate_arguments(CMAKE_C_FLAGS)
separate_arguments(CMAKE_C_FLAGS_DEBUG)
separate_arguments(CMAKE_C_FLAGS_RELEASE)
separate_arguments(CMAKE_C_FLAGS_RELWITHDEBINFO)
separate_arguments(CMAKE_C_FLAGS_MINSIZEREL)
separate_arguments(CMAKE_CXX_FLAGS)
separate_arguments(CMAKE_CXX_FLAGS_DEBUG)
separate_arguments(CMAKE_CXX_FLAGS_RELEASE)
separate_arguments(CMAKE_CXX_FLAGS_RELWITHDEBINFO)
separate_arguments(CMAKE_CXX_FLAGS_MINSIZEREL)

set(PRJ_C_FLAGS                  "${CMAKE_C_FLAGS}"                  CACHE STRING "overridden flags")
set(PRJ_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG}"            CACHE STRING "overridden flags")
set(PRJ_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE}"          CACHE STRING "overridden flags")
set(PRJ_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO}"   CACHE STRING "overridden flags")
set(PRJ_C_FLAGS_MINSIZEREL       "${CMAKE_C_FLAGS_MINSIZEREL}"       CACHE STRING "overridden flags")
set(PRJ_CXX_FLAGS                "${CMAKE_CXX_FLAGS}"                CACHE STRING "overridden flags")
set(PRJ_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}"          CACHE STRING "overridden flags")
set(PRJ_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}"        CACHE STRING "overridden flags")
set(PRJ_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "overridden flags")
set(PRJ_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}"     CACHE STRING "overridden flags")

set(CMAKE_C_FLAGS                  "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_C_FLAGS_DEBUG            "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_C_FLAGS_RELEASE          "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL       "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_CXX_FLAGS                "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG          "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE        "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "" CACHE STRING "overridden flags" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL     "" CACHE STRING "overridden flags" FORCE)

set(PRJ_ALL_C_FLAGS $<$<COMPILE_LANGUAGE:C>:${PRJ_C_FLAGS};$<$<CONFIG:Debug>:${PRJ_C_FLAGS_DEBUG}>;$<$<CONFIG:Release>:${CMAKE_C_FLAGS_RELEASE}>;$<$<CONFIG:RelWithDebInfo>:${CMAKE_C_FLAGS_RELWITHDEBINFO}>;$<$<CONFIG:MinSizeRel>:${CMAKE_C_FLAGS_MINSIZEREL}>>)
set(PRJ_ALL_CXX_FLAGS $<$<COMPILE_LANGUAGE:CXX>:${PRJ_CXX_FLAGS};$<$<CONFIG:Debug>:${PRJ_CXX_FLAGS_DEBUG}>;$<$<CONFIG:Release>:${CMAKE_CXX_FLAGS_RELEASE}>;$<$<CONFIG:RelWithDebInfo>:${CMAKE_CXX_FLAGS_RELWITHDEBINFO}>;$<$<CONFIG:MinSizeRel>:${CMAKE_CXX_FLAGS_MINSIZEREL}>>)
set(PRJ_ALL_COMBINED_FLAGS ${PRJ_ALL_C_FLAGS};${PRJ_ALL_CXX_FLAGS})

# ----------------------------------------------------------------------------------
# figure out for which platform we target and what we are running on

# targets
if(EMSCRIPTEN)
	set(PRJ_TARGET_EMSCRIPTEN		1)
	set(PRJ_SHADERS_PLATFORM		"asm.js")
	set(PRJ_BUILD_EXECUTABLE		1)
	#set(PRJ_FMOD_AVAILABLE			1)

	link_directories(
		${ROOT}/3rdparty/bgfx/libs/emscripten
	)

elseif(ANDROID_PLATFORM)
	set(PRJ_TARGET_ANDROID			1)
	set(PRJ_SHADERS_PLATFORM		"android")
	set(PRJ_BUILD_SHARED_LIB		1)
	#set(PRJ_FMOD_AVAILABLE			1)

	# TODO fix android CPU arch build
	link_directories(
		${ROOT}/3rdparty/bgfx/libs/android_armv7a
		${ROOT}/3rdparty/fmod/libs/android/armeabi-v7a
	)

elseif(APPLE AND IOS)
	set(PRJ_TARGET_IOS				1)
	set(PRJ_SHADERS_PLATFORM		"ios")

elseif(APPLE)
	set(PRJ_TARGET_MACOS 			1)
	set(PRJ_SHADERS_PLATFORM 		"osx")
	set(PRJ_BUILD_EXECUTABLE		1)
	#set(PRJ_FMOD_AVAILABLE			1)

	link_directories(
		${ROOT}/3rdparty/bgfx/libs/osx_x64
		${ROOT}/3rdparty/fmod/libs/osx
	)

	if(NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		message(FATAL_ERROR "only 64 bit builds are supported")
	endif()

elseif(WIN32)
	set(PRJ_TARGET_WINDOWS			1)
	set(PRJ_SHADERS_PLATFORM		"windows")
	set(PRJ_SHADERS_ARGS_VS			-p vs_4_0 -O 3)
	set(PRJ_SHADERS_ARGS_FS			-p ps_4_0 -O 3)
	set(PRJ_BUILD_EXECUTABLE		1)
	#set(PRJ_FMOD_AVAILABLE			1)

	link_directories(
		${ROOT}/3rdparty/bgfx/libs/vs2015_x64
		${ROOT}/3rdparty/fmod/libs/vs2015_x64
		#${ROOT}/3rdparty/nanomsg/libs/vs2015_x64
	)

	if(NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		message(FATAL_ERROR "only 64 bit builds are supported")
	endif()

	# hotfix bgfx debug build for non release builds on Windows, because of STL
	if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
		set(BGFX_DEBUG 1)
	endif()
else()
	message(FATAL_ERROR "unknown target")
endif()

if(FMOD_DISABLE)
	unset(PRJ_FMOD_AVAILABLE)
endif()

# hosts
if(CMAKE_HOST_APPLE)
	set(PRJ_HOST_MACOS			1)
	set(PRJ_SHADERS_COMPILER	"${ROOT}/3rdparty/bgfx/bin/shaderc_osx")
	set(PRJ_TEXTURE_COMPILER	"${ROOT}/3rdparty/bgfx/bin/texturec_osx")
	set(PRJ_BIN2C				"${ROOT}/3rdparty/bin2c/bin2c")
	set(PRJ_PYTHON				"/usr/bin/python")

elseif(CMAKE_HOST_WIN32)
	set(PRJ_HOST_WINDOWS		1)
	set(PRJ_SHADERS_COMPILER	"${ROOT}/3rdparty/bgfx/bin/shaderc_win.exe")
	set(PRJ_TEXTURE_COMPILER	"${ROOT}/3rdparty/bgfx/bin/texturec_win.exe")
	set(PRJ_BIN2C				"${ROOT}/3rdparty/bin2c/bin2c.exe")
	if(PRJ_TARGET_EMSCRIPTEN)
		# Emscripten overrides python but we still want to use system python because of installed packages
		set(PRJ_PYTHON			py -3)
	else()
		set(PRJ_PYTHON			"python")
	endif()

else()
	message(FATAL_ERROR "unknown host")
endif()

# ----------------------------------------------------------------------------------
# built-in shaders and textures

set(gen_src)

list(APPEND gen_src ${ROOT}/src/shaders/tex_color_vs_${PRJ_SHADERS_PLATFORM}.h)
add_custom_command(
	OUTPUT ${ROOT}/src/shaders/tex_color_vs_${PRJ_SHADERS_PLATFORM}.h
	COMMAND ${PRJ_SHADERS_COMPILER}
	-i 3rdparty/bgfx/include --type vertex --platform ${PRJ_SHADERS_PLATFORM} ${PRJ_SHADERS_ARGS_VS}
	-f src/shaders/tex_color.vs -o src/shaders/tex_color_vs_${PRJ_SHADERS_PLATFORM}.h --bin2c tex_color_vs
	DEPENDS src/shaders/tex_color.vs ${PRJ_SHADERS_COMPILER}
	WORKING_DIRECTORY ${ROOT}
)

list(APPEND gen_src ${ROOT}/src/shaders/tex_color_fs_${PRJ_SHADERS_PLATFORM}.h)
add_custom_command(
	OUTPUT ${ROOT}/src/shaders/tex_color_fs_${PRJ_SHADERS_PLATFORM}.h
	COMMAND ${PRJ_SHADERS_COMPILER}
	-i 3rdparty/bgfx/include --type fragment --platform ${PRJ_SHADERS_PLATFORM} ${PRJ_SHADERS_ARGS_FS}
	-f src/shaders/tex_color.fs -o src/shaders/tex_color_fs_${PRJ_SHADERS_PLATFORM}.h --bin2c tex_color_fs
	DEPENDS src/shaders/tex_color.fs ${PRJ_SHADERS_COMPILER}
	WORKING_DIRECTORY ${ROOT}
)

list(APPEND gen_src ${ROOT}/src/_missing_texture.h)
add_custom_command(
	OUTPUT ${ROOT}/src/_missing_texture.h
	COMMAND ${PRJ_TEXTURE_COMPILER} -f src/_missing_texture.png -o src/_missing_texture.ktx -t RGBA8
	COMMAND ${PRJ_BIN2C} -n _missing_texture -o src/_missing_texture.h src/_missing_texture.ktx
	DEPENDS src/_missing_texture.png ${PRJ_TEXTURE_COMPILER} ${PRJ_BIN2C}
	WORKING_DIRECTORY ${ROOT}
)

# ----------------------------------------------------------------------------------
# build sources

file(GLOB_RECURSE src
	${ROOT}/3rdparty/bgfx/include/*.h
	${ROOT}/3rdparty/entrypoint/*.h
	${ROOT}/3rdparty/entrypoint/*.c
	#${ROOT}/3rdparty/fmod/include/*.h
	${ROOT}/3rdparty/fontstash/*.h
	${ROOT}/3rdparty/gb/*.h
	${ROOT}/3rdparty/gb/*.c
	${ROOT}/3rdparty/nativefonts/*.h
	${ROOT}/3rdparty/nativefonts/*.c
	${ROOT}/3rdparty/nativefonts/*.cpp
	${ROOT}/3rdparty/stb/*.h
	${ROOT}/3rdparty/stb/*.c
	${ROOT}/3rdparty/tinycthread/*.h
	${ROOT}/3rdparty/tinycthread/*.c
	#${ROOT}/3rdparty/tlsf/*.h
	#${ROOT}/3rdparty/tlsf/*.c
	${ROOT}/src/*.h
	${ROOT}/src/*.c
	${ROOT}/examples/*.c
	${ROOT}/examples/*.h
)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ROOT}/src ${ROOT}/examples)

if(PRJ_TARGET_IOS OR PRJ_TARGET_MACOS)
	file(GLOB_RECURSE src_objc
		${ROOT}/3rdparty/entrypoint/*.m
		${ROOT}/src/*.m
	)
	list(APPEND src ${src_objc})
endif()

list(APPEND src ${gen_src})

if(PRJ_BUILD_EXECUTABLE)
	add_executable(${PRJ_TARGET} ${src})
elseif(PRJ_BUILD_SHARED_LIB)
	add_library(${PRJ_TARGET} SHARED ${src})
else()
	# in case if we're not building final executable here
	# always build all generated files
	add_custom_target(${PRJ_TARGET}_builtins ALL DEPENDS ${src})
endif()

if(PRJ_BUILD_EXECUTABLE OR PRJ_BUILD_SHARED_LIB)
	target_compile_options(${PRJ_TARGET} PRIVATE ${PRJ_ALL_COMBINED_FLAGS})

	target_include_directories(
		${PRJ_TARGET} PRIVATE
		${ROOT}/3rdparty/bgfx/include
		${ROOT}/3rdparty/entrypoint
		#${ROOT}/3rdparty/fmod/include
		${ROOT}/3rdparty/fontstash
		${ROOT}/3rdparty/gb
		${ROOT}/3rdparty/nativefonts
		${ROOT}/3rdparty/stb
		${ROOT}/3rdparty/tinycthread
		#${ROOT}/3rdparty/tlsf
		${ROOT}/src/helpers
		${ROOT}/src/shaders
		${ROOT}/src
		${ROOT}/exmaples/00-helloworld
	)

	target_compile_definitions(${PRJ_TARGET} PRIVATE
		SHADER_INCLUDE_VS=\"tex_color_vs_${PRJ_SHADERS_PLATFORM}.h\"
		SHADER_INCLUDE_FS=\"tex_color_fs_${PRJ_SHADERS_PLATFORM}.h\"
	)

	if(BGFX_DEBUG)
		target_link_libraries(
			${PRJ_TARGET}
			bgfxDebug
			bimgDebug
			bxDebug
		)
	else()
		target_link_libraries(
			${PRJ_TARGET}
			bgfxRelease
			bimgRelease
			bxRelease
		)
	endif()

	if(PRJ_FMOD_AVAILABLE)
		target_compile_definitions(${PRJ_TARGET} PRIVATE FMOD_AVAILABLE)
		if(FMOD_DEBUG)
			target_compile_definitions(${PRJ_TARGET} PRIVATE FMOD_DEBUG)
		endif()
		if(PRJ_TARGET_WINDOWS)
			target_link_libraries(${PRJ_TARGET} fmod64_vc fmodstudio64_vc)
		elseif(PRJ_TARGET_EMSCRIPTEN)
			set(FMOD_EMSCRIPTEN_LIBS "${ROOT}/3rdparty/fmod/libs/emscripten/fmod.bc ${ROOT}/3rdparty/fmod/libs/emscripten/fmodstudio.bc")
		else()
			target_link_libraries(${PRJ_TARGET} fmod fmodstudio)
		endif()
	endif()

	if(PRJ_TARGET_EMSCRIPTEN)
		target_compile_options(${PRJ_TARGET} PUBLIC "-fomit-frame-pointer" "-O3" "-s" "AGGRESSIVE_VARIABLE_ELIMINATION=1" "-g0")
		set_target_properties(${PRJ_TARGET} PROPERTIES LINK_FLAGS "${FMOD_EMSCRIPTEN_LIBS} -fno-rtti -fno-exceptions -s TOTAL_MEMORY=256000000 --preload-file ${ROOT}/res@/ --memory-init-file 1 -O3")
		add_custom_command(TARGET ${PRJ_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ROOT}/emscripten-template/index.html" $<TARGET_FILE_DIR:${PRJ_TARGET}>)

	elseif(PRJ_TARGET_ANDROID)
		target_compile_options(${PRJ_TARGET} PUBLIC "-O3")
		target_include_directories(${PRJ_TARGET} PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue)
		target_link_libraries(${PRJ_TARGET} android log EGL GLESv2)

	elseif(PRJ_TARGET_IOS)
		target_compile_options(${PRJ_TARGET} PUBLIC "-fobjc-arc")

	elseif(PRJ_TARGET_MACOS)
		target_compile_options(${PRJ_TARGET} PUBLIC "-fobjc-arc")
		target_link_libraries(${PRJ_TARGET} "-framework Cocoa" "-framework CoreGraphics" "-framework Quartz" "-framework Metal")
		if(PRJ_FMOD_AVAILABLE)
			add_custom_command(TARGET ${PRJ_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ROOT}/3rdparty/fmod/libs/osx/libfmod.dylib" $<TARGET_FILE_DIR:${PRJ_TARGET}>)
			add_custom_command(TARGET ${PRJ_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ROOT}/3rdparty/fmod/libs/osx/libfmodstudio.dylib" $<TARGET_FILE_DIR:${PRJ_TARGET}>)
		endif()

	elseif(PRJ_TARGET_WINDOWS)
		target_compile_definitions(${PRJ_TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
		target_compile_definitions(${PRJ_TARGET} PRIVATE _ITERATOR_DEBUG_LEVEL=0)
		target_compile_definitions(${PRJ_TARGET} PRIVATE _WIN32)
		if(PRJ_FMOD_AVAILABLE)
			add_custom_command(TARGET ${PRJ_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ROOT}/3rdparty/fmod/bin/fmod64.dll" $<TARGET_FILE_DIR:${PRJ_TARGET}>)
			add_custom_command(TARGET ${PRJ_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ROOT}/3rdparty/fmod/bin/fmodstudio64.dll" $<TARGET_FILE_DIR:${PRJ_TARGET}>)
		endif()
	endif()
endif()
