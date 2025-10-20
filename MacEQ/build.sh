#!/bin/bash

# MacEQ Build Script
# Usage: ./build.sh [clean|build|run|release]

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project configuration
PROJECT_NAME="MacEQ"
SCHEME="MacEQ"
PROJECT_FILE="MacEQ.xcodeproj"
CONFIGURATION="Debug"
BUILD_DIR="build"

# Function to print colored output
print_info() {
    echo -e "${BLUE}ℹ ${NC}$1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

# Function to clean build artifacts
clean_build() {
    print_info "Cleaning build artifacts..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Removed build directory"
    fi
    
    xcodebuild clean \
        -project "$PROJECT_FILE" \
        -scheme "$SCHEME" \
        -configuration "$CONFIGURATION" \
        > /dev/null 2>&1
    
    print_success "Project cleaned successfully"
}

# Function to build the project
build_project() {
    local config=$1
    print_info "Building $PROJECT_NAME ($config configuration)..."
    
    xcodebuild build \
        -project "$PROJECT_FILE" \
        -scheme "$SCHEME" \
        -configuration "$config" \
        -derivedDataPath "$BUILD_DIR" \
        CODE_SIGN_IDENTITY="-" \
        CODE_SIGNING_REQUIRED=NO \
        CODE_SIGNING_ALLOWED=NO \
        | xcpretty || xcodebuild build \
        -project "$PROJECT_FILE" \
        -scheme "$SCHEME" \
        -configuration "$config" \
        -derivedDataPath "$BUILD_DIR" \
        CODE_SIGN_IDENTITY="-" \
        CODE_SIGNING_REQUIRED=NO \
        CODE_SIGNING_ALLOWED=NO
    
    print_success "Build completed successfully"
}

# Function to run the app
run_app() {
    print_info "Looking for built application..."
    
    APP_PATH=$(find "$BUILD_DIR" -name "${PROJECT_NAME}.app" -type d | head -n 1)
    
    if [ -z "$APP_PATH" ]; then
        print_error "Application not found. Building first..."
        build_project "$CONFIGURATION"
        APP_PATH=$(find "$BUILD_DIR" -name "${PROJECT_NAME}.app" -type d | head -n 1)
    fi
    
    if [ -n "$APP_PATH" ]; then
        print_success "Found app at: $APP_PATH"
        print_info "Launching $PROJECT_NAME..."
        open "$APP_PATH"
        print_success "Application launched"
    else
        print_error "Could not find application bundle"
        exit 1
    fi
}

# Function to build release version
build_release() {
    print_info "Building Release version..."
    CONFIGURATION="Release"
    build_project "Release"
    
    APP_PATH=$(find "$BUILD_DIR" -name "${PROJECT_NAME}.app" -type d | head -n 1)
    if [ -n "$APP_PATH" ]; then
        print_success "Release build available at: $APP_PATH"
    fi
}

# Function to show build info
show_info() {
    print_info "MacEQ Build Information"
    echo ""
    echo "Project: $PROJECT_NAME"
    echo "Scheme: $SCHEME"
    echo "Configuration: $CONFIGURATION"
    echo "Build Directory: $BUILD_DIR"
    echo ""
    
    if command -v xcodebuild &> /dev/null; then
        XCODE_VERSION=$(xcodebuild -version | head -n 1)
        print_success "Xcode: $XCODE_VERSION"
    else
        print_error "Xcode command line tools not found"
        exit 1
    fi
    
    if command -v xcpretty &> /dev/null; then
        print_success "xcpretty: installed (for prettier build output)"
    else
        print_warning "xcpretty: not installed (optional, install with: gem install xcpretty)"
    fi
}

# Function to show usage
show_usage() {
    echo "Usage: ./build.sh [command]"
    echo ""
    echo "Commands:"
    echo "  clean      Clean build artifacts"
    echo "  build      Build the project (Debug)"
    echo "  run        Build and run the application"
    echo "  release    Build release version"
    echo "  info       Show build information"
    echo "  help       Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./build.sh              # Build and run (default)"
    echo "  ./build.sh build        # Just build"
    echo "  ./build.sh clean build  # Clean then build"
    echo "  ./build.sh release      # Build release version"
}

# Check if xcpretty is available (optional, for prettier output)
if ! command -v xcpretty &> /dev/null; then
    print_warning "For better build output, install xcpretty: gem install xcpretty"
fi

# Main script logic
main() {
    echo ""
    echo "======================================"
    echo "   MacEQ - System Audio Equalizer    "
    echo "======================================"
    echo ""
    
    if [ $# -eq 0 ]; then
        # Default action: build and run
        build_project "$CONFIGURATION"
        run_app
        exit 0
    fi
    
    # Process commands
    while [ $# -gt 0 ]; do
        case $1 in
            clean)
                clean_build
                ;;
            build)
                build_project "$CONFIGURATION"
                ;;
            run)
                run_app
                ;;
            release)
                build_release
                ;;
            info)
                show_info
                ;;
            help|--help|-h)
                show_usage
                exit 0
                ;;
            *)
                print_error "Unknown command: $1"
                show_usage
                exit 1
                ;;
        esac
        shift
    done
    
    echo ""
    print_success "All tasks completed!"
}

# Run main function
main "$@"

