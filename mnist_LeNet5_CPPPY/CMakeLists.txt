
##### define path
set(LIB_PATH_INCLUDE "/home/wangyinzhi/thridlibs/include")
set(LIB_PATH_LIB "/home/wangyinzhi/thridlibs/lib")

##### include
include_directories(.)
include_directories(./AlgCore)
include_directories(./CommMgr)
include_directories(./Common)
include_directories(/home/wangyinzhi/tensorflow_build/opencv-3.4.6_src/install_cuda_tbb/include)
include_directories(${LIB_PATH_INCLUDE}/TNLog)
include_directories(${LIB_PATH_INCLUDE}/TNHttp)
include_directories(${LIB_PATH_INCLUDE}/)
include_directories(/home/wangyinzhi/tensorflow_build/TensorRT-5.1.5.0/include)
include_directories(/root/.pyenv/versions/anaconda3-5.1.0/include/python3.6m)
include_directories(/home/wangyinzhi/tensorflow_build/cuda9.0/include)

##### source
aux_source_directory(. SRC)
aux_source_directory(./AlgCore SRC)
aux_source_directory(./CommMgr SRC)
aux_source_directory(./Common SRC)

set(CUDA_TOOLKIT_ROOT_DIR /home/wangyinzhi/tensorflow_build/cuda9.0)
find_package(CUDA REQUIRED)
if (NOT CUDA_FOUND)
    message(WARNING "CUDA not found. Please install CUDA first.")
else ()
    include_directories(${CUDA_INCLUDE_DIRS})
endif ()


# OpenCV
set(OpenCV_DIR "/home/wangyinzhi/tensorflow_build/opencv-3.4.6_src/install_cuda_tbb/share/OpenCV")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

##### link
link_directories(
                # /usr/lib64
                 ${LIB_PATH_LIB}/TNLog
                 ${LIB_PATH_LIB}/TNHttp
                 ${LIB_PATH_LIB}/log4cplus
                 ${LIB_PATH_LIB}/jsoncpp
                 /root/.pyenv/versions/anaconda3-5.1.0/lib
                 /home/wangyinzhi/tensorflow_build/TensorRT-5.1.5.0/lib
                 /home/wangyinzhi/tensorflow_build/cuda9.0/lib64
                 /home/wangyinzhi/tensorflow_build/opencv-3.4.6_src/install_cuda_tbb/lib64
)

##### output
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "1.0.0")

configure_file(./AlgCore/evaluate.py ${PROJECT_BINARY_DIR}/bin/evaluate.py COPYONLY)
configure_file(./AlgCore/forward.py ${PROJECT_BINARY_DIR}/bin/forward.py COPYONLY)
configure_file(./AlgCore/testing.py ${PROJECT_BINARY_DIR}/bin/testing.py COPYONLY)
configure_file(./AlgCore/train.py ${PROJECT_BINARY_DIR}/bin/train.py COPYONLY)

target_link_libraries(${PROJECT_NAME} python3.6m log4cplus TNLog tn_http jsoncpp 
nvcaffe_parser nvinfer_plugin nvinfer nvonnxparser_runtime nvonnxparser nvparsers 
#tiff ${OpenCV_LIBS} ${CUDA_LIBRARIES})
/lib64/libtiff.so opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs opencv_dnn opencv_cudev 
 ${CUDA_LIBRARIES})

