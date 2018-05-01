# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

#配置so库信息
add_library( # Sets the name of the library.
             compress_image  #声明引用so库的名称 ，生成的so库就是这个名字

             # Sets the library as a shared library.
             SHARED  #表示共享so库文件

             #STATIC：静态库，是目标文件的归档文件，在链接其它目标的时候使用。
             #SHARED：动态库，会被动态链接，在运行时被加载。
             #MODULE：模块库，是不会被链接到其它目标中的插件，但是可能会在运行时使用dlopen-系列的函数动态链接。

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             #src/main/cpp/native-lib.cpp   #构建so库的源文件
             src/main/cpp/compress_image.cpp
             )



# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
#这个方法与我们要创建的so库无关而是使用NDK的Apis或者库
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

#寻找jnigraphics库，这个可以不写
find_library( jnigraphics-lib
                jnigraphics )

#导入jpeg动态库 SHARED；静态库为STATIC
add_library(jpeg STATIC IMPORTED)

#对应so目录
set_target_properties(jpeg PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libjpeg.so)


#如果你本地的库（native-lib/其他）想要调用log/jpeg库的方法，那么就需要配置这个属性，意思是把NDK库关联到本地库。
target_link_libraries( # Specifies the target library.
                       compress_image
                        jpeg
                       # Links the target library to the log library
                       # included in the NDK.
                       jnigraphics  #使用AndroidBitmap中的方法必须也要依赖Bitmap库：jnigraphics
                       ${log-lib} # 链接的库，根据log-lib变量对应liblog.so函数库
                       )
