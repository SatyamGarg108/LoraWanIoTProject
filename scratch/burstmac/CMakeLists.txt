file(GLOB burstmac_src "src/*.cc")
file(GLOB burstmac_headers "src/*.h")

build_exec(
    EXECNAME burstmac-simulation
    SOURCE_FILES examples/burstmac-simulation.cc ${burstmac_src} ${burstmac_headers}
    LIBRARIES_TO_LINK ${ns3-libs}
)
