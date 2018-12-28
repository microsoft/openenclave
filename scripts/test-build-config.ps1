# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

##====================================================================================
##
## This script fires OE build and test for specified build-type
## Default run with no parameters builds with Debug build-type and for SGX1
## platform and will test in Simulator mode.
## Please note that this script does not install any packages needed for build/test.
## Please install all packages necessary for your test before invoking this script.
## For CI runs, the Docker image will contain the necessary packages.
##
##====================================================================================

[CmdletBinding()]
Param
(
    [Switch]$help,
    [Switch]$ADD_WINDOWS_ENCLAVE_TESTS,
    
    # Valid BUILDTYPE values are Debug|Release
    [ValidateSet("Debug", "Release", IgnoreCase = $false)]
    [String]$BUILD_TYPE = "DEBUG",
    [ValidateSet("1", "")]
    [String]$BUILD_ENCLAVES = "",
    [Parameter(Mandatory = $true)][String]$LINUX_BIN_DIR
)

if ($h -or $help) {
     echo "Script to fire OE build and test with specified build-type/test mode" 
     echo " Usage: "
     echo " ./scripts/test-build-config.ps1"
     echo "        -help to Display usage and exit"
     echo "        -add_windows_enclave_tests to add tests for windows enclave"
     echo "        -build_type Debug|Release"
     echo "        -linux_bin_dir [directory] directory for linux binaries"
     echo " Default is to build for SGX1 platform, Debug Build type & test in"
     echo " simulator mode"
     echo ""
     exit 0
}

$ErrorActionPreference = "Stop"

$VS_PATH = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

echo "linux_bin_dir: $LINUX_BIN_DIR"

echo "BUILD_TYPE = $BUILD_TYPE"

# Delete the build directory if it exists. This allows calling this script iteratively
# for multiple configurations for a platform.
if (Test-Path "./build" -PathType Container) {
    Remove-Item -Path ./build -Recurse -Force
}

mkdir build
cd build

$BUILD_GENERATOR="Visual Studio 15 2017 Win64"
$LINUX_BIN_FLAG="-DLINUX_BIN_DIR=\"$LINUX_BIN_DIR\""
if ($BUILD_ENCLAVES -eq "1") {
    $BUILD_GENERATOR="NMake Makefiles"
    $LINUX_BIN_FLAG=""

    # Currently disable Windows Enclave Tests for BUILD_ENCLAVE builds.
    # This will be enabled in a later PR.
    Remove-Variable ADD_WINDOWS_ENCLAVE_TESTS
}

& $VS_PATH

where cl

if ($ADD_WINDOWS_ENCLAVE_TESTS) {
    & cmake.exe -G $BUILD_GENERATOR $LINUX_BIN_FLAG -DADD_WINDOWS_ENCLAVE_TESTS=1 -DBUILD_ENCLAVES=$BUILD_ENCLAVES ..
} else {
    & cmake.exe -G $BUILD_GENERATOR $LINUX_BIN_FLAG -DBUILD_ENCLAVES=$BUILD_ENCLAVES ..
}
if ($LASTEXITCODE) {
    echo ""
    echo "cmake failed"
    echo ""
    exit 1
}

if ($LASTEXITCODE) {
    echo ""
    echo "Visual Studio failed"
    echo ""
    exit 1
}

# Build
cmake.exe --build . --config $BUILD_TYPE
if ($LASTEXITCODE) {
    echo ""
    echo "Build failed"
    echo ""
    exit 1
}

ctest.exe -V -C $BUILD_TYPE
if ($LASTEXITCODE) {
    echo ""
    echo "Test failed for $BUILD_TYPE"
    echo ""
    exit 1
}
