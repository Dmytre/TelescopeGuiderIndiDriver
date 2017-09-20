include_directories( /usr/include/libindi )

add_executable(MountGuidingDrv Guider.cpp COMInteract.cpp)

target_link_libraries(MountGuidingDrv indidriver)
target_link_libraries(MountGuidingDrv z)
target_link_libraries(MountGuidingDrv nova)
