## CMake helper for the Burst-MAC example module
## This file collects the burstmac sources and registers a runnable
## example executable `burstmac-simulation` for use with the ns-3 build.

file(GLOB burstmac_src "src/*.cc")
file(GLOB burstmac_headers "src/*.h")

# Define the example executable that builds with the ns-3 infrastructure
build_exec(
    EXECNAME burstmac-simulation
    SOURCE_FILES examples/burstmac-simulation.cc ${burstmac_src} ${burstmac_headers}
    LIBRARIES_TO_LINK ${ns3-libs}
    # Ensure the executable is written into the build output tree (not /)
    EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/burstmac
)
