#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008  stepping stone GmbH
#| 
#| This program is free software; you can redistribute it and/or
#| modify it under the terms of the GNU General Public License
#| Version 2 as published by the Free Software Foundation.
#| 
#| This program is distributed in the hope that it will be useful,
#| but WITHOUT ANY WARRANTY; without even the implied warranty of
#| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#| GNU General Public License for more details.
#| 
#| You should have received a copy of the GNU General Public License
#| along with this program; if not, write to the Free Software
#| Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

cmake_minimum_required(VERSION 2.8.8)

# If available, enable developer warnings for the build system
if(POLICY CMP0005)
    cmake_policy(SET CMP0005 NEW)
endif(POLICY CMP0005)

IF(NOT SSBACKUP_EXECUTABLE_NAME)
  SET(SSBACKUP_EXECUTABLE_NAME sepiola)
ENDIF(NOT SSBACKUP_EXECUTABLE_NAME)

IF(NOT SSBACKUP_APPLICATION_FULL_NAME)
	SET(SSBACKUP_APPLICATION_FULL_NAME "Sepiola Online Backup Client")
ENDIF(NOT SSBACKUP_APPLICATION_FULL_NAME)

PROJECT(${SSBACKUP_EXECUTABLE_NAME})

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
INCLUDE(Macros)
INCLUDE(TargetArch)

# enable CTest module
ENABLE_TESTING()

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(Qt4_FIND_REQUIRED TRUE)
INCLUDE(FindQt4)

# Translatable source files
FILE(GLOB_RECURSE translate_files src/*.cc src/*.hh src/*.ui)

SET(CPACK_PACKAGE_NAME ${SSBACKUP_EXECUTABLE_NAME})
SET(CPACK_PACKAGE_FULL_NAME ${SSBACKUP_APPLICATION_FULL_NAME})
SET(CPACK_PACKAGE_VENDOR "stepping stone GmbH")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_FULL_NAME})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CPACK_NSIS_QT_SUFFIX "d")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

# Has to be below the variables because they are accessed in the src/CMakeLists.txt
ADD_SUBDIRECTORY(src)

SET(CPACK_SSBACKUP_EXECUTABLE_NAME       "${SSBACKUP_EXECUTABLE_NAME}")
SET(CPACK_PROJECT_SOURCE_DIR             "${PROJECT_SOURCE_DIR}")
SET(CPACK_SSBACKUP_APPLICATION_FULL_NAME "${SSBACKUP_APPLICATION_FULL_NAME}")

target_architecture(TARGET_ARCH)

IF(Q_WS_WIN)
	SET(CPACK_GENERATOR "NSIS")
	IF(TARGET_ARCH STREQUAL "x86_64")
		SET(WINDOWS_NAME win64)
	ELSE()
		SET(WINDOWS_NAME win32)
	ENDIF()
	SET(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${WINDOWS_NAME})
	SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/installer/win32-x86/" ${CMAKE_MODULE_PATH})
	INCLUDE(CPack)
ELSEIF(Q_WS_MAC)
	ADD_CUSTOM_TARGET(package
			  COMMAND rm -rf ${CMAKE_BINARY_DIR}/staging
                          COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/staging -P cmake_install.cmake
                          COMMAND hdiutil create -ov -srcfolder ${CMAKE_BINARY_DIR}/staging/${PROJECT_NAME} ${SSBACKUP_EXECUTABLE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-Mac-universal.dmg)
ELSE(Q_WS_WIN)
	EXECUTE_PROCESS(COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
	SET(CPACK_GENERATOR "STGZ")
	SET(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-Linux-${TARGET_ARCH})
	SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/" ${CMAKE_MODULE_PATH})
	INCLUDE(CPack)
ENDIF(Q_WS_WIN)
