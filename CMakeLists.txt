cmake_minimum_required(VERSION 3.0)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_SYSROOT /home/z/embed/arm-himix200-linux/target)

set(tools /home/z/embed/arm-himix200-linux)
set(CMAKE_C_COMPILER ${tools}/bin/arm-himix200-linux-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-himix200-linux-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(Myvdec VERSION 1.0.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions(-w) # close warning

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(Folly REQUIRED)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC -rdynamic -fpermissive -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fPIC -rdynamic -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-int-conversion -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a7")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a7")

include_directories(${CMAKE_SYSROOT}/usr/include ${CMAKE_CURRENT_LIST_DIR})
link_directories(${CMAKE_SYSROOT}/usr/lib)
add_executable(Myvdec sample_vdec.cpp   Vdec.cpp Vdec.h HisiRtsp.h HisiRtsp.cpp RtspImpl.h)
target_link_libraries(Myvdec 
opencv_core
opencv_imgproc
opencv_highgui
opencv_imgcodecs
opencv_videoio
opencv_video
opencv_calib3d
opencv_features2d
pthread
mpi
ive
md
nnie
hdmi
_hiae
isp
_hidehaze
_hidrc
_hildci
_hiawb
sns_imx327
sns_imx327_2l
sns_imx307
sns_imx458
sns_mn34220
sns_os05a
VoiceEngine
securec
sns_imx335
upvqe
dnvqe)
