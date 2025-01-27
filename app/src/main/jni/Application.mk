#目标平台ABI类型
APP_ABI = armeabi armeabi-v7a arm64-v8a x86 x86_64

#运行库类型
#system(default) 系统默认的C++运行库。是最小支持的C++运行库
#c++_shared 以动态链接方式使用的LLVM libc++
APP_STL = c++_shared