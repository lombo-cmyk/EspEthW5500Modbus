#set(PROJECT_NAME "EspEthW5500Modbus")
set(CMAKE_CXX_STANDARD 17)

idf_component_register(SRCS "main/src/EthernetW5500.cpp" "main/src/Free.cpp" "main/src/Modbus.cpp"
        INCLUDE_DIRS "main/include"
        REQUIRES CppSingletonTemplate
        PRIV_REQUIRES freemodbus)