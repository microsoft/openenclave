# !/bin/bash
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#
# script to test the samples
#
# Arguments: [-i <install_prefix> <bin_dir>]
#
# -i <bin_dir> <install_prefix> <tmp_prefix>]
#   Temporary install prior to test-run, use this for verification inside
#   the build tree
#
#    <bin_dir> - PROJECT_BINARY_DIR from cmake
#    <install_prefix> - the CMAKE_INSTALL_PREFIX prefix configured w/ cmake
#    <tmp_prefix> - absolute path to put the tmp install under
#    <USE_LIBSGX=1> - Indicates that USE_LIBSGX is set for this run
#
# run the make-variant with make
# run the cmake-variant in separate cmake instance

printandexit(){
    echo An error occured
    exit 1
}

# Collect arguments and do temporary install if so requested
if test "$1" = "-i" ; then
    # inside build tree. install using DESTDIR mechanism.
    BIN_DIR=$(realpath $2)
    INSTALL_DIR=$(realpath -m $4$3)
    rm -rf "$INSTALL_DIR"
    make -C "$BIN_DIR" "DESTDIR=$4" install
else
    # inside installed tree. Assume this is placed under
    # prefix/share/openenclave/samples/
    INSTALL_DIR=$(realpath $(dirname $0)/../../..)
fi || printandexit

TEST_MAKE_DIR="$INSTALL_DIR/share/openenclave/samples"

# build & run the make samples
make -C "$TEST_MAKE_DIR" OPENENCLAVE_CONFIG="$INSTALL_DIR/share/openenclave/samples/config.mak" OE_PREFIX=$INSTALL_DIR world "USE_LIBSGX=$5" || printandexit

exit 0
