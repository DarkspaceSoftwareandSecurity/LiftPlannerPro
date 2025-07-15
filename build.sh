#!/bin/bash

# AutoCAD Clone Build Script
# Comprehensive build system for Qt 6 + OpenCASCADE CAD application

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_DIR="install"
PARALLEL_JOBS=$(nproc)
VERBOSE=false
CLEAN=false
INSTALL=false

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    cat << EOF
AutoCAD Clone Build Script

Usage: $0 [OPTIONS]

OPTIONS:
    -h, --help          Show this help message
    -d, --debug         Build in Debug mode (default: Release)
    -c, --clean         Clean build directory before building
    -i, --install       Install after building
    -j, --jobs N        Number of parallel jobs (default: $PARALLEL_JOBS)
    -v, --verbose       Verbose output
    --build-dir DIR     Build directory (default: $BUILD_DIR)
    --install-dir DIR   Install directory (default: $INSTALL_DIR)

EXAMPLES:
    $0                  # Build in Release mode
    $0 -d -v            # Build in Debug mode with verbose output
    $0 -c -i            # Clean build and install
    $0 -j 8             # Build with 8 parallel jobs

DEPENDENCIES:
    - Qt 6.2 or later
    - OpenCASCADE 7.6 or later
    - CMake 3.20 or later
    - C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --install-dir)
            INSTALL_DIR="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check CMake
    if ! command_exists cmake; then
        print_error "CMake not found. Please install CMake 3.20 or later."
        exit 1
    fi
    
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    print_status "Found CMake $CMAKE_VERSION"
    
    # Check compiler
    if command_exists g++; then
        GCC_VERSION=$(g++ --version | head -n1 | cut -d' ' -f4)
        print_status "Found GCC $GCC_VERSION"
    elif command_exists clang++; then
        CLANG_VERSION=$(clang++ --version | head -n1 | cut -d' ' -f4)
        print_status "Found Clang $CLANG_VERSION"
    else
        print_error "No suitable C++ compiler found. Please install GCC 10+ or Clang 12+."
        exit 1
    fi
    
    # Check Qt 6
    if command_exists qmake6; then
        QT_VERSION=$(qmake6 -query QT_VERSION)
        print_status "Found Qt $QT_VERSION"
    elif command_exists qmake; then
        QT_VERSION=$(qmake -query QT_VERSION)
        if [[ $QT_VERSION == 6.* ]]; then
            print_status "Found Qt $QT_VERSION"
        else
            print_warning "Found Qt $QT_VERSION, but Qt 6.2+ is required"
        fi
    else
        print_warning "Qt qmake not found in PATH. Make sure Qt 6 is installed."
    fi
    
    print_success "Dependency check completed"
}

# Function to clean build directory
clean_build() {
    if [ -d "$BUILD_DIR" ]; then
        print_status "Cleaning build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    fi
}

# Function to configure build
configure_build() {
    print_status "Configuring build..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # CMake configuration
    CMAKE_ARGS=(
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_INSTALL_PREFIX="../$INSTALL_DIR"
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    )
    
    # Add verbose flag if requested
    if [ "$VERBOSE" = true ]; then
        CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
    fi
    
    # Try to find Qt 6 automatically
    if command_exists qmake6; then
        QT_DIR=$(qmake6 -query QT_INSTALL_PREFIX)
        CMAKE_ARGS+=(-DQt6_DIR="$QT_DIR/lib/cmake/Qt6")
    fi
    
    print_status "Running CMake with arguments: ${CMAKE_ARGS[*]}"
    
    if ! cmake "${CMAKE_ARGS[@]}" ..; then
        print_error "CMake configuration failed"
        exit 1
    fi
    
    cd ..
    print_success "Build configured successfully"
}

# Function to build project
build_project() {
    print_status "Building project..."
    
    cd "$BUILD_DIR"
    
    BUILD_ARGS=(--build . --config "$BUILD_TYPE" --parallel "$PARALLEL_JOBS")
    
    if [ "$VERBOSE" = true ]; then
        BUILD_ARGS+=(--verbose)
    fi
    
    print_status "Building with $PARALLEL_JOBS parallel jobs..."
    
    if ! cmake "${BUILD_ARGS[@]}"; then
        print_error "Build failed"
        exit 1
    fi
    
    cd ..
    print_success "Build completed successfully"
}

# Function to install project
install_project() {
    print_status "Installing project..."
    
    cd "$BUILD_DIR"
    
    if ! cmake --install . --config "$BUILD_TYPE"; then
        print_error "Installation failed"
        exit 1
    fi
    
    cd ..
    print_success "Installation completed successfully"
}

# Function to show build summary
show_summary() {
    print_status "Build Summary:"
    echo "  Build Type: $BUILD_TYPE"
    echo "  Build Directory: $BUILD_DIR"
    echo "  Install Directory: $INSTALL_DIR"
    echo "  Parallel Jobs: $PARALLEL_JOBS"
    
    if [ -f "$BUILD_DIR/AutoCADClone" ] || [ -f "$BUILD_DIR/AutoCADClone.exe" ]; then
        print_success "Executable built successfully!"
        
        if [ "$INSTALL" = true ] && [ -d "$INSTALL_DIR" ]; then
            print_status "Installation location: $(pwd)/$INSTALL_DIR"
        else
            print_status "Executable location: $(pwd)/$BUILD_DIR"
        fi
    fi
}

# Main execution
main() {
    print_status "AutoCAD Clone Build Script"
    print_status "=========================="
    
    # Check dependencies
    check_dependencies
    
    # Clean if requested
    if [ "$CLEAN" = true ]; then
        clean_build
    fi
    
    # Configure build
    configure_build
    
    # Build project
    build_project
    
    # Install if requested
    if [ "$INSTALL" = true ]; then
        install_project
    fi
    
    # Show summary
    show_summary
    
    print_success "All operations completed successfully!"
}

# Run main function
main "$@"
