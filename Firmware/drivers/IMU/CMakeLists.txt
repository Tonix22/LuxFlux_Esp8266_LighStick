set(srcs "imu6050.c" "SimpleIMU-6.cpp")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include")




