#set(PROJECT_NAME "EspEthW5500Modbus")
set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "tcp_slave.cpp" "EthernetW5500.cpp" "include/Free.cpp"
                    INCLUDE_DIRS ".")
