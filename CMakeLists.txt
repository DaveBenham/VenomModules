cmake_minimum_required(VERSION 3.22)

if (DEFINED ENV{METAMODULE_SDK_DIR})
    set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
elseif("${METAMODULE_SDK_DIR}" STREQUAL "")
    set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../metamodule-plugin-sdk")
endif()
include(${METAMODULE_SDK_DIR}/plugin.cmake)

project(VenomMetaModulePlugin 
    VERSION     0.1
    DESCRIPTION "Venom Plugin for MetaModule"
    LANGUAGES   C CXX ASM
)

add_library(Venom STATIC)

# convenient variable pointing to the VCV Rack repo
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

target_sources(Venom
    PRIVATE
    ${SOURCE_DIR}/src/BenjolinGatesExpander.cpp
    ${SOURCE_DIR}/src/BenjolinOsc.cpp
    ${SOURCE_DIR}/src/BenjolinVoltsExpander.cpp
    ${SOURCE_DIR}/src/BernoulliSwitch.cpp
    ${SOURCE_DIR}/src/BernoulliSwitchExpander.cpp
    ${SOURCE_DIR}/src/HQ.cpp
    ${SOURCE_DIR}/src/Knob5.cpp
    ${SOURCE_DIR}/src/LinearBeats.cpp
    ${SOURCE_DIR}/src/LinearBeatsExpander.cpp
    ${SOURCE_DIR}/src/Logic.cpp
    ${SOURCE_DIR}/src/Mix4.cpp
    ${SOURCE_DIR}/src/Mix4Stereo.cpp
    ${SOURCE_DIR}/src/MixFade.cpp
    ${SOURCE_DIR}/src/MixFade2.cpp
    ${SOURCE_DIR}/src/MixMute.cpp
    ${SOURCE_DIR}/src/MixOffset.cpp
    ${SOURCE_DIR}/src/MixPan.cpp
    ${SOURCE_DIR}/src/MixSend.cpp
    ${SOURCE_DIR}/src/MixSolo.cpp
    ${SOURCE_DIR}/src/MultiMerge.cpp
    ${SOURCE_DIR}/src/MultiSplit.cpp
    ${SOURCE_DIR}/src/NORS-IQ.cpp
    ${SOURCE_DIR}/src/Oscillator.cpp
    ${SOURCE_DIR}/src/PolyFade.cpp
    ${SOURCE_DIR}/src/PolySHASR.cpp
    ${SOURCE_DIR}/src/Push5.cpp
    ${SOURCE_DIR}/src/QuadVCPolarizer.cpp
    ${SOURCE_DIR}/src/Recurse.cpp
    ${SOURCE_DIR}/src/RecurseStereo.cpp
    ${SOURCE_DIR}/src/Reformation.cpp
    ${SOURCE_DIR}/src/RhythmExplorer.cpp
    ${SOURCE_DIR}/src/ShapedVCA.cpp
    ${SOURCE_DIR}/src/Thru.cpp
    ${SOURCE_DIR}/src/VCAMix4.cpp
    ${SOURCE_DIR}/src/VCAMix4Stereo.cpp
    ${SOURCE_DIR}/src/VCOUnit.cpp
    ${SOURCE_DIR}/src/WaveFolder.cpp
    ${SOURCE_DIR}/src/WinComp.cpp

    ${SOURCE_DIR}/src/plugin.cpp
)

target_include_directories(Venom PRIVATE
    ${SOURCE_DIR}
)

target_compile_options(Venom
    PRIVATE
    -Wno-deprecated
    -Wno-deprecated-enum-float-conversion
    # No flags needed
)

set_property(TARGET Venom PROPERTY CXX_STANDARD 20)

if("${INSTALL_DIR}" STREQUAL "")
    set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins)
endif()

create_plugin(
    SOURCE_LIB      Venom                     # This is the cmake target name (defined in add_target)
    PLUGIN_NAME     Venom                     # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${SOURCE_DIR}/plugin.json   # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${SOURCE_DIR}/assets        # Path to the assets/ dir containing the PNGs
    DESTINATION     ${INSTALL_DIR}              # Path to where you want the plugin file output
)
