set(include_dirs "include")

set(srcs "Light_effects.cpp" 
         "Menu.cpp" 
         "memory_admin.cpp")


idf_component_register(SRCS "${srcs}" 
                      INCLUDE_DIRS "${include_dirs}")
