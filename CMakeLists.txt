cmake_minimum_required (VERSION 3.10.2)
project (Vessel)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# The version number.
set (Vessel_VERSION_MAJOR 1)
set (Vessel_VERSION_MINOR 0)

#Custom CMake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
	"${PROJECT_SOURCE_DIR}/VesselConfig.h.in"
	"${PROJECT_BINARY_DIR}/VesselConfig.h"
)

# add the binary tree to the search path for include files
# so that we will find VesselConfig.h
include_directories("${PROJECT_BINARY_DIR}")

#Include directories for compilation
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/client)
set(VESSEL_SRC_DIR "${PROJECT_SOURCE_DIR}/src/vessel")

###############################################
## Vessel Client Executable
###############################################
add_executable(
	vessel
	client/client.cpp
	${VESSEL_SRC_DIR}/aws/aws_s3_client.cpp
	${VESSEL_SRC_DIR}/azure/azure_client.cpp
#	${VESSEL_SRC_DIR}/compression/compress.cpp ${VESSEL_SRC_DIR}/compression/tarball.cpp
	${VESSEL_SRC_DIR}/crypto/hash_util.cpp
	${VESSEL_SRC_DIR}/database/local_db.cpp
	${VESSEL_SRC_DIR}/filesystem/directory.cpp ${VESSEL_SRC_DIR}/filesystem/file.cpp ${VESSEL_SRC_DIR}/filesystem/file_iterator.cpp ${VESSEL_SRC_DIR}/filesystem/file_upload.cpp
	${VESSEL_SRC_DIR}/log/log.cpp
	${VESSEL_SRC_DIR}/network/http_client.cpp ${VESSEL_SRC_DIR}/network/http_request.cpp ${VESSEL_SRC_DIR}/network/http_stream.cpp
	${VESSEL_SRC_DIR}/vessel/queue_manager.cpp ${VESSEL_SRC_DIR}/vessel/upload_aws.cpp ${VESSEL_SRC_DIR}/vessel/upload_azure.cpp ${VESSEL_SRC_DIR}/vessel/upload_interface.cpp ${VESSEL_SRC_DIR}/vessel/upload_manager.cpp ${VESSEL_SRC_DIR}/vessel/upload_vessel.cpp ${VESSEL_SRC_DIR}/vessel/vessel_client.cpp
	${VESSEL_SRC_DIR}/vessel/app_manager.cpp ${VESSEL_SRC_DIR}/vessel/stat_manager.cpp
)

#OS Dependent Libs
if(WIN32)
	target_link_libraries(vessel ws2_32 wsock32 userenv)
else()
	target_link_libraries(vessel ${CMAKE_DL_LIBS})
endif()

#Threading
set(CMAKE_THREAD_PREFER_PTHREAD true)
set(THREADS_PREFER_PTHREAD_FLAG true)
find_package(Threads)
if (Threads_FOUND)
	target_link_libraries(vessel ${CMAKE_THREAD_LIBS_INIT})
else()
	message (FATAL_ERROR "Could not find Threads package")
endif()

#ZLib
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
	target_link_libraries(vessel ${ZLIB_LIBRARIES})
else()
	   message (FATAL_ERROR "Cannot find ZLib")
endif()

#LibArchive
#[[
find_package(LibArchive REQUIRED)
if(LibArchive_FOUND)
	include_directories(${LibArchive_INCLUDE_DIRS})
	target_link_libraries(vessel ${LibArchive_LIBRARIES})
else()
	   message (FATAL_ERROR "Cannot find LibArchive")
endif()
]]

###############################################
## Boost Libs
###############################################
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.68.0 REQUIRED COMPONENTS system filesystem date_time thread log log_setup program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
		target_link_libraries(vessel ${Boost_LIBRARIES})
else()
    message (FATAL_ERROR "Cannot find Boost")
endif()

###############################################
## SQLite
###############################################
find_package(sqlite3)
if (SQLITE3_FOUND)
  include_directories(${SQLITE3_INCLUDE_DIRS})
  target_link_libraries (vessel ${SQLITE3_LIBRARIES})
else()
    message (FATAL_ERROR "Cannot find sqlite3")
endif()

###############################################
## OpenSSL
###############################################
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
	include_directories(${OPENSSL_INCLUDE_DIR})
	target_link_libraries(vessel OpenSSL::SSL OpenSSL::Crypto)
else()
	message (FATAL_ERROR "Cannot find OpenSSL")
endif()

###############################################
## CryptoPP
###############################################
find_package(CryptoPP REQUIRED)
if(CryptoPP_FOUND)
	target_link_libraries(vessel ${CRYPTOPP_LIBRARIES})
	include_directories(${CRYPTOPP_INCLUDE_DIRS})
else()
	message (FATAL_ERROR "Cannot find CryptoPP")
endif()

###############################################
## Rapidjson
###############################################
find_package(rapidjson)
if (RAPIDJSON_FOUND)
  include_directories(${RAPIDJSON_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find rapidjson")
endif()

###############################################
## Vessel Libs
###############################################
#[[
add_library(AwsS3Client_static STATIC ${VESSEL_SRC_DIR}/aws/aws_s3_client.cpp)
add_library(AwsS3Client SHARED ${VESSEL_SRC_DIR}/aws/aws_s3_client.cpp)
#
add_library(AwsUpload_static STATIC ${VESSEL_SRC_DIR}/vessel/upload_aws.cpp)
add_library(AwsUpload SHARED ${VESSEL_SRC_DIR}/vessel/upload_aws.cpp)
#
add_library(AzureClient_static STATIC ${VESSEL_SRC_DIR}/azure/azure_client.cpp)
add_library(AzureClient SHARED ${VESSEL_SRC_DIR}/azure/azure_client.cpp)
#
add_library(AzureUpload_static STATIC ${VESSEL_SRC_DIR}/vessel/upload_azure.cpp)
add_library(AzureUpload SHARED ${VESSEL_SRC_DIR}/vessel/upload_azure.cpp)
#
#add_library(Compress_static STATIC ${VESSEL_SRC_DIR}/compression/compress.cpp)
#add_library(Compress SHARED ${VESSEL_SRC_DIR}/compression/compress.cpp)
#
#add_library(Tarball_static STATIC ${VESSEL_SRC_DIR}/compression/tarball.cpp)
#add_library(Tarball SHARED ${VESSEL_SRC_DIR}/compression/tarball.cpp)
#
add_library(Hash_static STATIC ${VESSEL_SRC_DIR}/crypto/hash_util.cpp)
add_library(Hash SHARED ${VESSEL_SRC_DIR}/crypto/hash_util.cpp)
#
add_library(LocalDatabase_static STATIC ${VESSEL_SRC_DIR}/database/local_db.cpp)
add_library(LocalDatabase SHARED ${VESSEL_SRC_DIR}/database/local_db.cpp)
#
add_library(BackupDirectory_static STATIC ${VESSEL_SRC_DIR}/filesystem/directory.cpp)
add_library(BackupDirectory SHARED ${VESSEL_SRC_DIR}/filesystem/directory.cpp)
#
add_library(BackupFile_static STATIC ${VESSEL_SRC_DIR}/filesystem/file.cpp)
add_library(BackupFile SHARED ${VESSEL_SRC_DIR}/filesystem/file.cpp)
#
add_library(FileIterator_static STATIC ${VESSEL_SRC_DIR}/filesystem/file_iterator.cpp)
add_library(FileIterator SHARED ${VESSEL_SRC_DIR}/filesystem/file_iterator.cpp)
#
add_library(FileUpload_static STATIC ${VESSEL_SRC_DIR}/filesystem/file_upload.cpp)
add_library(FileUpload SHARED ${VESSEL_SRC_DIR}/filesystem/file_upload.cpp)
#
add_library(Log_static STATIC ${VESSEL_SRC_DIR}/log/log.cpp)
add_library(Log SHARED ${VESSEL_SRC_DIR}/log/log.cpp)
#
add_library(HttpClient_static STATIC ${VESSEL_SRC_DIR}/network/http_client.cpp)
add_library(HttpClient SHARED ${VESSEL_SRC_DIR}/network/http_client.cpp)
#
add_library(HttpRequest_static STATIC ${VESSEL_SRC_DIR}/network/http_request.cpp)
add_library(HttpRequest SHARED ${VESSEL_SRC_DIR}/network/http_request.cpp)
#
add_library(HttpStream_static STATIC ${VESSEL_SRC_DIR}/network/http_stream.cpp)
add_library(HttpStream SHARED ${VESSEL_SRC_DIR}/network/http_stream.cpp)
#
add_library(QueueManager_static STATIC ${VESSEL_SRC_DIR}/vessel/queue_manager.cpp)
add_library(QueueManager SHARED ${VESSEL_SRC_DIR}/vessel/queue_manager.cpp)
#
add_library(UploadInterface_static STATIC ${VESSEL_SRC_DIR}/vessel/upload_interface.cpp)
add_library(UploadInterface SHARED ${VESSEL_SRC_DIR}/vessel/upload_interface.cpp)
#
add_library(UploadManager_static STATIC ${VESSEL_SRC_DIR}/vessel/upload_manager.cpp)
add_library(UploadManager SHARED ${VESSEL_SRC_DIR}/vessel/upload_manager.cpp)
#
add_library(VesselUpload_static STATIC ${VESSEL_SRC_DIR}/vessel/upload_vessel.cpp)
add_library(VesselUpload SHARED ${VESSEL_SRC_DIR}/vessel/upload_vessel.cpp)
#
add_library(VesselClient_static STATIC ${VESSEL_SRC_DIR}/vessel/vessel_client.cpp)
add_library(VesselClient SHARED ${VESSEL_SRC_DIR}/vessel/vessel_client.cpp)
]]

# add the install targets
install (TARGETS vessel DESTINATION ${CMAKE_BINARY_DIR}/bin)
