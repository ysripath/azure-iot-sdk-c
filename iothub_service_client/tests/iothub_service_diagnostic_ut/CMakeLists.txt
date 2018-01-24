#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for iothub_devicetwin_ut
cmake_minimum_required(VERSION 2.8.11)

compileAsC11()

set(theseTestsName iothub_service_diagnostic_ut)

set(${theseTestsName}_test_files
iothub_service_diagnostic_ut.c
)


set(${theseTestsName}_c_files
../../src/iothub_service_diagnostic.c
../../../deps/parson/parson.c
)

set(${theseTestsName}_h_files
../../../deps/parson/parson.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/UnitTests")
