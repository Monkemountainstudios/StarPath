cmake_minimum_required(VERSION 3.22)

project(Starpath VERSION 0.1.0 LANGUAGES C CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(STARPATH_JUCE_SOURCE_DIR "" CACHE PATH "Path to a JUCE checkout")
if(NOT STARPATH_JUCE_SOURCE_DIR)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
        set(STARPATH_JUCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    elseif(WIN32 AND EXISTS "F:/JUCE/CMakeLists.txt")
        set(STARPATH_JUCE_SOURCE_DIR "F:/JUCE")
    else()
        message(FATAL_ERROR "JUCE was not found. Pass -DSTARPATH_JUCE_SOURCE_DIR=/path/to/JUCE")
    endif()
endif()

add_subdirectory("${STARPATH_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

set(STARPATH_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND STARPATH_FORMATS AU)
endif()

juce_add_binary_data(StarpathAssets
    HEADER_NAME StarpathAssets.h
    NAMESPACE StarpathAssets
    SOURCES Assets/starfield.jpg)

juce_add_plugin(Starpath
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "MonkeMountainStudios"
    COMPANY_WEBSITE "https://monkemountainstudios.neocities.org"
    BUNDLE_ID "com.monkemountainstudios.starpath"
    PLUGIN_MANUFACTURER_CODE Mnke
    PLUGIN_CODE StrP
    FORMATS ${STARPATH_FORMATS}
    PRODUCT_NAME "Starpath"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Assets/StarpathIcon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Assets/StarpathIcon.png")

juce_generate_juce_header(Starpath)

target_sources(Starpath PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/StarpathEngine.cpp
    Source/StarpathEngine.h)

target_compile_definitions(Starpath PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

if(WIN32)
    target_compile_definitions(Starpath PUBLIC JUCE_ASIO=1)
endif()

target_link_libraries(Starpath PRIVATE
    StarpathAssets
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

juce_add_console_app(StarpathDspTests PRODUCT_NAME "Starpath DSP Tests")
juce_generate_juce_header(StarpathDspTests)
target_sources(StarpathDspTests PRIVATE
    Tests/StarpathEngineTest.cpp
    Source/StarpathEngine.cpp
    Source/StarpathEngine.h)
target_include_directories(StarpathDspTests PRIVATE Source)
target_compile_definitions(StarpathDspTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(StarpathDspTests PRIVATE juce::juce_audio_basics juce::juce_core juce::juce_dsp)
add_test(NAME StarpathDspSmoke COMMAND StarpathDspTests)


