cmake_minimum_required(VERSION 3.10)
project(burstmac)

# Find all source files
file(GLOB_RECURSE SOURCES "*.cc" "*.h")

# Create executable
# Note: In NS-3 scratch context, we usually rely on the parent CMakeLists.txt to find us.
# But since we are in a subdirectory, we need to be careful.
# The parent CMakeLists.txt in scratch/ scans subdirectories and adds them if they have CMakeLists.txt.

# We need to link against NS-3 libraries.
# The variable ${ns3-libs} is available from the parent scope.

add_executable(burstmac-simulation ${SOURCES})
target_link_libraries(burstmac-simulation ${ns3-libs})
