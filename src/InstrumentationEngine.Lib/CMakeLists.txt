# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# Require at least version 2.8.12 of CMake
cmake_minimum_required(VERSION 2.8.12)
project(InstrumentationEngine.Lib)
find_package(LibXml2 REQUIRED)

include_directories(. ${LIBXML2_INCLUDE_DIR})

build_init(CPP InstrumentationEngine.Lib)

set(src_files
    ./stdafx.cpp
    ./ConfigurationLoader.cpp
    ./ConfigurationLocator.cpp
    ./ConfigurationSource.cpp
    ./DataContainer.cpp
    ./Encoding.cpp
    ./FileLoggerSink.cpp
    ./HostLoggerSink.cpp
    ./ImplQueryInterface.cpp
    ./InstrumentationMethod.cpp
    ./InstrumentationMethodAttachContext.cpp
    ./InstrumentationMethodSetting.cpp
    ./LoggerService.cpp
    ./Logging.cpp
    ./refcount.cpp
    ./StringUtils.cpp
    ./XmlDocWrapper.cpp
    ./XmlNode.cpp
)

add_lib(${PROJECT_NAME}
    STATIC
    true # use_redefines
    false # hide_symbols
    ${src_files}
    )