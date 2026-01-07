add_library(railway_logic
    logic/Interlocking.cpp
    logic/ControllerHelpers.cpp
    logic/ControllerLogic.cpp
)

target_include_directories(railway_logic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(railway_logic PUBLIC
    # Add any dependencies here
)