# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             libjpeg_utils

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/libjpeg_utils.cpp )
#设置distribution_DIR变量
#${CMAKE_SOURCE_DIR}为调用系统预先设定好的变量或者方法，这里返回CMakeLists文件地址
set(distribution_DIR   ${CMAKE_SOURCE_DIR}/libs/)
# Specifies a path to native header files.
include_directories(src/main/cpp/include/)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
find_library( # Sets the name of the path variable.
              android-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              android )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
#添加我们编译好的静态库到工程
#libjpeg.a
add_library(libjpeg-lib STATIC IMPORTED)
set_target_properties(libjpeg-lib  PROPERTIES IMPORTED_LOCATION
${distribution_DIR}/${ANDROID_ABI}/libjpeg.so)

target_link_libraries( # Specifies the target library.
                       libjpeg_utils

                       # Links the target library to the log library
                       # included in the NDK.
                       #这里的作用保证AndroidBitmap_getInfo方法可用。你也可以通过find_library方法再添加
                       jnigraphics
                       ${android-lib}
                       ${distribution_DIR}/${ANDROID_ABI}/libjpeg.so
                       libjpeg-lib
                       ${log-lib} )