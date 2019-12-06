##### define path
set(LIB_PATH_INCLUDE "/home/wangyinzhi/thridlibs/include")
set(LIB_PATH_LIB "/home/wangyinzhi/thridlibs/lib")

# CUDA
set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")

# TensorRT
set(TRT_INCLUDE_PATH "/home/wangyinzhi/thridlibs/TensorRT-5.1.5.0/include")
set(TRT_LIB_PATH "/home/wangyinzhi/thridlibs/TensorRT-5.1.5.0/lib")

# OpenCV
set(OpenCV_DIR "/usr/local/opencv/share/OpenCV")

# CUDA
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

# TensorRT
include_directories(${TRT_INCLUDE_PATH})
link_directories(${TRT_LIB_PATH})
set(TRT_LIBS nvcaffe_parser nvinfer_plugin nvinfer nvonnxparser_runtime nvonnxparser nvparsers)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

##### include
include_directories(.)
include_directories(./AlgCore)
include_directories(./CommMgr)
include_directories(./Common)
include_directories(${LIB_PATH_INCLUDE}/TNLog)
include_directories(${LIB_PATH_INCLUDE}/TNHttp)
include_directories(${LIB_PATH_INCLUDE}/)
include_directories(/root/.pyenv/versions/anaconda3-5.1.0/include/python3.6m)

##### source
aux_source_directory(. SRC)
aux_source_directory(./AlgCore SRC)
aux_source_directory(./CommMgr SRC)
aux_source_directory(./Common SRC)

##### link
link_directories(
                # /usr/lib64
                 ${LIB_PATH_LIB}/TNLog
                 ${LIB_PATH_LIB}/TNHttp
                 ${LIB_PATH_LIB}/log4cplus
                 ${LIB_PATH_LIB}/jsoncpp
                 /root/.pyenv/versions/anaconda3-5.1.0/lib
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
/lib64/libtiff.so opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs opencv_dnn opencv_cudev 
 ${CUDA_LIBRARIES} ${OpenCV_LIBS} ${TRT_LIBS} 
 /root/.pyenv/versions/anaconda3-5.1.0/pkgs/cudnn-7.6.4-cuda10.1_0/lib/libcudnn.so.7.6.4)

