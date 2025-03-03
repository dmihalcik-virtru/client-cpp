###########################################################
#
# Copyright 2019 Virtru Corporation
#
# SPDX - License Identifier: BSD-3-Clause-Clear
#
# Core library
###########################################################
cmake_minimum_required (VERSION 3.8)

# enable verbose logging
set(CMAKE_VERBOSE_MAKEFILE ON)

#get version 
file(STRINGS ../../VERSION PKG_VERSION)

# Set the project name
project(opentdf VERSION ${PKG_VERSION})

configure_file(${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_SOURCE_DIR}/include/version.h)

# Add suffix to differentiate between shared and static lib output names
set(TDF_STATIC_LIB_SUFFIX _static)
set(TDF_COMBINED_LIB_SUFFIX _combined)

set(CMAKE_CXX_STANDARD 17)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add code coverage if requested
if("$ENV{VBUILD_CODE_COVERAGE}" STREQUAL "true")
    message(STATUS "Adding code coverage compilation flag")
    add_compile_options(--coverage)
else()
    message(STATUS "Skipping code coverage compilation flag")
endif()

# add source files
#
# Please make all updates to the source file list to CMakeSources.txt
set (TDF_CPP_SDK_SOURCE_DIR ${PROJECT_SOURCE_DIR})
include(${PROJECT_SOURCE_DIR}/CMakeSources.txt)
set(SOURCE_FILES ${TDF_CPP_SDK_SOURCE_FILES})

#############################################################
## Add tao json library - is a zero-dependency C++17 header-only JSON library
## Add jwt library - Depends on boringssl
#############################################################
set(TAO_CPP_JSON_DIR ${PROJECT_SOURCE_DIR}/../thirdparty-libs/common)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/crypto
    ${PROJECT_SOURCE_DIR}/include/network
    ${PROJECT_SOURCE_DIR}/src
    ${TAO_CPP_JSON_DIR}
    ${CONAN_INCLUDE_DIRS}
    )

# enable warning
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WLIB lib)
    set(ZLIB zlib)
    set(LIBARCHIVE "${CONAN_LIBS_LIBARCHIVE}.lib")
    set(LIBXML2 libxml2_a)
    set(OTHERLIBS crypt32 bcrypt)
    if($ENV{VBUILDMODE} MATCHES $ENV{VBUILDDEBUG})
        set(DEBUGSUFFIX d)
    endif()
else()
    set(ZLIB z)
    set(LIBXML2 xml2)
    set(LIBARCHIVE "lib${CONAN_LIBS_LIBARCHIVE}.a")
    add_compile_options(-Wall -Wextra -fPIC)
    if(DEFINED ENV{VBUILD_ADD_MACOS_FLAGS})
      if($ENV{VBUILD_ADD_MACOS_FLAGS} MATCHES "true")
        add_compile_options(-Wno-string-plus-int)
        add_compile_options(-Wno-deprecated-declarations)
      endif()
    endif()
    if(DEFINED ENV{VBUILD_ADD_RPI_FLAGS})
      if($ENV{VBUILD_ADD_RPI_FLAGS} MATCHES "true")
        add_compile_options(-mfloat-abi=hard -mfpu=vfpv2)
      endif()
    endif()
endif()

# set link info
link_directories(
    ${CONAN_LIB_DIRS}
    )

link_libraries(
    ${WLIB}boost_regex
    ${WLIB}boost_iostreams
    ${WLIB}crypto${DEBUGSUFFIX}
    ${WLIB}ssl${DEBUGSUFFIX}
    ${CONAN_LIBS_LIBARCHIVE}
    ${LIBXML2}
    iconv
    ${ZLIB}
    bz2
    ${OTHERLIBS}
    )

# generate the static library from the library sources
add_library(${PROJECT_NAME}static STATIC ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME}static PROPERTIES OUTPUT_NAME ${PROJECT_NAME}${TDF_STATIC_LIB_SUFFIX})
set_target_properties(${PROJECT_NAME}static PROPERTIES COMPILE_FLAGS "-DLIBARCHIVE_STATIC=1")

get_target_property(TDF_STATIC_LIB_NAME ${PROJECT_NAME}static OUTPUT_NAME)
set(TDF_STATIC_LIB_FILENAME ${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}${TDF_STATIC_LIB_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
message(STATUS "Static lib name: ${TDF_STATIC_LIB_FILENAME}")
set(TDF_STATIC_LIB_FULL_PATH ${CMAKE_BINARY_DIR}/lib/${TDF_STATIC_LIB_FILENAME})
message(STATUS "Static lib full path: ${TDF_STATIC_LIB_FULL_PATH}")

set(TDF_COMBINED_LIB_FILENAME ${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}${TDF_STATIC_LIB_SUFFIX}${TDF_COMBINED_LIB_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
set(TDF_COMBINED_LIB_FULL_PATH ${CMAKE_BINARY_DIR}/lib/${TDF_COMBINED_LIB_FILENAME})
message(STATUS "Combined full path: ${TDF_COMBINED_LIB_FULL_PATH}")

#TODO: Also need to handle iOS and Android(arm64).
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    add_custom_command(TARGET ${PROJECT_NAME}static POST_BUILD
            COMMAND libtool -static -o ${TDF_COMBINED_LIB_FULL_PATH} ${TDF_STATIC_LIB_FULL_PATH}
            ${CONAN_LIB_DIRS_BOOST}/libboost_regex.a
            ${CONAN_LIB_DIRS_BOOST}/libboost_iostreams.a
            ${CONAN_LIB_DIRS_OPENSSL}/libcrypto.a
            ${CONAN_LIB_DIRS_OPENSSL}/libssl.a
            ${CONAN_LIB_DIRS_ZLIB}/libz.a
            ${CONAN_LIB_DIRS_BZIP2}/libbz2.a
            ${CONAN_LIB_DIRS_LIBXML2}/libxml2.a
            ${CONAN_LIB_DIRS_LIBICONV}/libiconv.a
            ${CONAN_LIB_DIRS_LIBARCHIVE}/${LIBARCHIVE}


            COMMENT "Combining libs and creating ${TDF_COMBINED_LIB_FILENAME}...")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    # TODO: Eventually replace ar command with libtool
    # Create a mri file for ar
    set(MRI_FILE  ${CMAKE_CURRENT_BINARY_DIR}/tdf.mri)

    file(WRITE ${MRI_FILE} "create  ${TDF_COMBINED_LIB_FULL_PATH}\n")
    file(APPEND ${MRI_FILE} "addlib ${TDF_STATIC_LIB_FULL_PATH}\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_BOOST}/libboost_regex.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_BOOST}/libboost_iostreams.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_OPENSSL}/libcrypto.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_OPENSSL}/libssl.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_ZLIB}/libz.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_BZIP2}/libbz2.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_LIBXML2}/libxml2.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_LIBICONV}/libiconv.a\n")
    file(APPEND ${MRI_FILE} "addlib ${CONAN_LIB_DIRS_LIBARCHIVE}/${LIBARCHIVE}\n")
    file(APPEND ${MRI_FILE} "save\n")
    file(APPEND ${MRI_FILE} "end\n")

    message(STATUS "mri file ${MRI_FILE}")
    add_custom_command(TARGET ${PROJECT_NAME}static POST_BUILD
            COMMAND  ar -M < ${MRI_FILE}
            COMMENT "Combining libs and creating ${TDF_COMBINED_LIB_FILENAME}...")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if(DEFINED ENV{VBUILDMODE})
      message(STATUS "Setting out dir based on VBUILDMODE")
      # special case for msvc with /Debug or /Release suffix
      if($ENV{VBUILDMODE} MATCHES $ENV{VBUILDRELEASE})
          set(MSVC_LIBRARY_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Release)
      else()
          set(MSVC_LIBRARY_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Debug)
  		set(DEBUGSUFFIX d)
      endif()
    else()
      # conan recipe build - VBUILDxxx is not set
      message(STATUS "Setting out dir based on conan recipe build")
      set(MSVC_LIBRARY_OUTPUT_DIR ${CMAKE_BINARY_DIR}/lib)
    endif()
    message(STATUS "msvc lib out path: ${MSVC_LIBRARY_OUTPUT_DIR}")
    set(TDF_STATIC_LIB_FULL_PATH_MSVC ${MSVC_LIBRARY_OUTPUT_DIR}/${TDF_STATIC_LIB_FILENAME})
    message(STATUS "msvc lib full path: ${TDF_STATIC_LIB_FULL_PATH_MSVC}")

    add_custom_command(TARGET ${PROJECT_NAME}static POST_BUILD
        COMMAND lib -out:${TDF_COMBINED_LIB_FULL_PATH} 
        ${TDF_STATIC_LIB_FULL_PATH_MSVC}
        ${CONAN_LIB_DIRS_OPENSSL}/${WLIB}crypto${DEBUGSUFFIX}.lib
        ${CONAN_LIB_DIRS_OPENSSL}/${WLIB}ssl${DEBUGSUFFIX}.lib
        crypt32.lib
        ws2_32.lib
        bcrypt.lib
        ${CONAN_LIB_DIRS_BOOST}/${WLIB}boost_regex.lib
        ${CONAN_LIB_DIRS_BOOST}/${WLIB}boost_iostreams.lib
        ${CONAN_LIB_DIRS_ZLIB}/zlib.lib
        ${CONAN_LIB_DIRS_BZIP2}/bz2.lib
        ${CONAN_LIB_DIRS_LIBXML2}/${WLIB}xml2_a.lib
        ${CONAN_LIB_DIRS_LIBICONV}/iconv.lib
        ${CONAN_LIB_DIRS_LIBARCHIVE}/${LIBARCHIVE}

        COMMENT "Combining libs and creating ${TDF_COMBINED_LIB_FILENAME}...")
else()
    message(STATUS "${CMAKE_SYSTEM_NAME} - unknown system FIXME........")
endif()

# generate the shared library from the library sources
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "Skipping shared lib on windows")
else()
  add_library(${PROJECT_NAME}shared SHARED ${SOURCE_FILES})
  set_target_properties(${PROJECT_NAME}shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
  get_target_property(TDF_SHARED_LIB_NAME ${PROJECT_NAME}shared OUTPUT_NAME)
  if(DEFINED ENV{VBUILD_ADD_RPI_FLAGS})
    if($ENV{VBUILD_ADD_RPI_FLAGS} MATCHES "true")
      set_target_properties(${PROJECT_NAME}shared PROPERTIES LINK_FLAGS "-mfloat-abi=hard -mfpu=vfpv2")
    endif()
  endif()
  set(TDF_SHARED_LIB_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${TDF_SHARED_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(TDF_SHARED_LIB_FULL_PATH ${CMAKE_BINARY_DIR}/lib/${TDF_SHARED_LIB_FILENAME})
  message(STATUS "Shared lib name: ${TDF_SHARED_LIB_FILENAME}")
  message(STATUS "Shared lib full path: ${TDF_SHARED_LIB_FULL_PATH}")
endif()

############################################################
# Package tdf lib for customer
############################################################

set(TDF_LIB_INSTALL_LOCATION  ${PROJECT_SOURCE_DIR}/../../tdf-lib-cpp)
install(DIRECTORY DESTINATION ${TDF_LIB_INSTALL_LOCATION})

# move the headers(to include) and tdf(to lib) directory under tdf-lib-cpp
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${TDF_LIB_INSTALL_LOCATION}/include)
        
install(FILES ${TDF_STATIC_LIB_FULL_PATH}
    DESTINATION ${TDF_LIB_INSTALL_LOCATION}/lib)

install(FILES ${TDF_COMBINED_LIB_FULL_PATH}
    DESTINATION ${TDF_LIB_INSTALL_LOCATION}/lib)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  message(STATUS "Skipping shared lib on windows")
else()
  install(FILES ${TDF_SHARED_LIB_FULL_PATH}
    DESTINATION ${TDF_LIB_INSTALL_LOCATION}/lib)
endif()

install(FILES ${PROJECT_SOURCE_DIR}/../../README.md DESTINATION ${TDF_LIB_INSTALL_LOCATION})
install(FILES ${PROJECT_SOURCE_DIR}/../../LICENSE DESTINATION ${TDF_LIB_INSTALL_LOCATION})
install(FILES ${PROJECT_SOURCE_DIR}/../../VERSION DESTINATION ${TDF_LIB_INSTALL_LOCATION})
        
