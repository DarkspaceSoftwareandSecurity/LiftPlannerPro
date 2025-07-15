cmake_minimum_required(VERSION 3.20)
project(AutoCADClone VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
find_package(OpenCASCADE REQUIRED)

# Set Qt6 specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(${OpenCASCADE_INCLUDE_DIR})
include_directories(src)
include_directories(src/ui)
include_directories(src/tools)
include_directories(src/geometry)
include_directories(src/commands)

# Source files
set(SOURCES
    src/main.cpp
    src/CADApplication.cpp
    src/MainWindow.cpp
    src/CommandManager.cpp
    src/GeometryEngine.cpp
    
    # UI Components
    src/ui/RibbonInterface.cpp
    src/ui/DockablePalettes.cpp
    src/ui/ViewportManager.cpp
    src/ui/StatusBar.cpp
    src/ui/ContextMenus.cpp
    src/ui/NavigationControls.cpp
    src/ui/ViewCube.cpp
    
    # 2D Drawing Tools
    src/tools/drawing/LineTools.cpp
    src/tools/drawing/CircleTools.cpp
    src/tools/drawing/ShapeTools.cpp
    src/tools/drawing/AnnotationTools.cpp
    src/tools/drawing/HatchTools.cpp
    
    # 3D Modeling Tools
    src/tools/modeling/SolidPrimitives.cpp
    src/tools/modeling/ExtrudeTools.cpp
    src/tools/modeling/BooleanOperations.cpp
    src/tools/modeling/SurfaceTools.cpp
    src/tools/modeling/MeshTools.cpp
    
    # Modification Tools
    src/tools/modification/TransformTools.cpp
    src/tools/modification/EditingTools.cpp
    src/tools/modification/ArrayTools.cpp
    
    # Organization & Management
    src/LayerManager.cpp
    src/BlockManager.cpp
    src/XrefManager.cpp
    src/LayoutManager.cpp
    
    # Object Snaps & Input Aids
    src/ObjectSnaps.cpp
    src/DynamicInput.cpp
    src/GripEditing.cpp
    src/PolarTracking.cpp
    
    # Visualization & Analysis
    src/RenderEngine.cpp
    src/MaterialSystem.cpp
    src/LightingSystem.cpp
    src/AnalysisTools.cpp
)

# Header files
set(HEADERS
    src/CADApplication.h
    src/MainWindow.h
    src/CommandManager.h
    src/GeometryEngine.h
    
    # UI Components
    src/ui/RibbonInterface.h
    src/ui/DockablePalettes.h
    src/ui/ViewportManager.h
    src/ui/StatusBar.h
    src/ui/ContextMenus.h
    src/ui/NavigationControls.h
    src/ui/ViewCube.h
    
    # 2D Drawing Tools
    src/tools/drawing/LineTools.h
    src/tools/drawing/CircleTools.h
    src/tools/drawing/ShapeTools.h
    src/tools/drawing/AnnotationTools.h
    src/tools/drawing/HatchTools.h
    
    # 3D Modeling Tools
    src/tools/modeling/SolidPrimitives.h
    src/tools/modeling/ExtrudeTools.h
    src/tools/modeling/BooleanOperations.h
    src/tools/modeling/SurfaceTools.h
    src/tools/modeling/MeshTools.h
    
    # Modification Tools
    src/tools/modification/TransformTools.h
    src/tools/modification/EditingTools.h
    src/tools/modification/ArrayTools.h
    
    # Organization & Management
    src/LayerManager.h
    src/BlockManager.h
    src/XrefManager.h
    src/LayoutManager.h
    
    # Object Snaps & Input Aids
    src/ObjectSnaps.h
    src/DynamicInput.h
    src/GripEditing.h
    src/PolarTracking.h
    
    # Visualization & Analysis
    src/RenderEngine.h
    src/MaterialSystem.h
    src/LightingSystem.h
    src/AnalysisTools.h
)

# Resource files
set(RESOURCES
    resources/icons.qrc
    resources/styles.qrc
)

# Create executable
add_executable(AutoCADClone ${SOURCES} ${HEADERS} ${RESOURCES})

# Link libraries
target_link_libraries(AutoCADClone
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${OpenCASCADE_LIBRARIES}
)

# Set target properties
set_target_properties(AutoCADClone PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Install target
install(TARGETS AutoCADClone
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Copy OpenCASCADE DLLs on Windows
if(WIN32)
    add_custom_command(TARGET AutoCADClone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OpenCASCADE_BINARY_DIR}/*.dll
        $<TARGET_FILE_DIR:AutoCADClone>
    )
endif()
