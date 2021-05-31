set(srcs "tcp_client.c" "tcp_server.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include")




