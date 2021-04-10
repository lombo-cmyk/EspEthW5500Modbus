#set(PROJECT_NAME "EspEthW5500Modbus")
set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "src/EthernetW5500.cpp" "src/Free.cpp" "src/Modbus.cpp"
        INCLUDE_DIRS "include")
