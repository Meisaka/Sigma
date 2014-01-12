# FindOVR.cmake
# A simple module for "finding" the Oculus VR files
# Copyright (c) 2013, Meisaka Yukara

if(NOT OVR_BASE)
	set(OVR_BASE "" CACHE PATH "Path to the LibOVR directory")
else(NOT OVR_BASE)
	INCLUDE_DIRECTORIES(${OVR_BASE}/Include)
endif(NOT OVR_BASE)

FIND_PATH(OVR_INCLUDE_DIR 
	NAMES OVR.h OVRVersion.h
	HINTS
	${OVR_BASE}/Include
	)

IF(CMAKE_CL_64)
SET(OVR_NAMES64 libovr64.lib libovr64d.lib)
ELSE(CMAKE_CL_64)
SET(OVR_NAMES64 "")
ENDIF(CMAKE_CL_64)

FIND_LIBRARY(OVR_LIBRARY 
	NAMES ${OVR_NAMES64} libovr libovr.a libovr.lib libovrd.lib
	PATH_SUFFIXES Debug Release Win32 x64 i386 x86_64
	HINTS 
	${OVR_BASE}/Lib
	${OVR_BASE}/Lib/Linux
	)

UNSET(OVR_NAMES64)

if(OVR_INCLUDE_DIR AND OVR_LIBRARY)
	set(OVR_FOUND TRUE)
endif(OVR_INCLUDE_DIR AND OVR_LIBRARY)

mark_as_advanced(OVR_INCLUDE_DIR OVR_LIBRARY)

if(OVR_FOUND)
	
else(OVR_FOUND)
	if(OVR_FIND_REQUIRED)
		if(NOT OVR_LIBRARY)
			message(FATAL_ERROR "The Oculus VR Library is not found")
		endif(NOT OVR_LIBRARY)
		if(NOT OVR_INCLUDE_DIR)
			message(FATAL_ERROR "The Oculus VR Include files are not found")
		endif(NOT OVR_INCLUDE_DIR)
	else(OVR_FIND_REQUIRED)
		set(OVR_LIBRARY "")
		message(STATUS "The Oculus VR components are not found")
	endif(OVR_FIND_REQUIRED)
endif(OVR_FOUND)
