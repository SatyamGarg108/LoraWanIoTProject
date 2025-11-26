# How to build and run the Burst-MAC example (ns-3)

## Prerequisites
- A working ns-3 tree (this workspace). Build dependencies installed for your platform.
- You are on Linux (bash). Adjust paths if you use a different shell.

---

## 1) Build the project

- From the project root (where `./ns3` wrapper is located), run:

```bash
cd /home/satyamwsl/IoT/ns-3-dev
./ns3
```

This runs the top-level build wrapper which configures and builds ns-3. If you prefer CMake directly:

```bash
mkdir -p cmake-cache
/usr/bin/cmake --build cmake-cache -j 5
```

**Notes:**
- If build fails, inspect the terminal output for the error. Common fixes: install missing dev packages, re-run `./ns3` after installing missing dependencies.

---

## 2) Locate the built example binary

- After a successful build the example binary for the Burst-MAC example is placed in `build/` (project root `build` directory). The target is named `burstmac-simulation`.
- Full path (example): `/home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation`

---

## 3) Prepare environment for running (library path)

- ns-3 shared libraries are in `build/lib`. Export `LD_LIBRARY_PATH` so the example can find them:

```bash
export LD_LIBRARY_PATH=/home/satyamwsl/IoT/ns-3-dev/build/lib:$LD_LIBRARY_PATH
```

---

## 4) Run a quick smoke test

- Run with small node count to verify it starts and finishes:

```bash
/home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation --nNodes=20 --nGateways=1 --burstPercent=50
```

---

## 5) Run with logging enabled

- To see component-level logs, use the `NS_LOG` environment variable. Example:

```bash
NS_LOG=BurstMacSimulation=info:BurstScheduler=info:BurstNodeApp=info \
LD_LIBRARY_PATH=/home/satyamwsl/IoT/ns-3-dev/build/lib:$LD_LIBRARY_PATH \
/home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation --nNodes=20 --nGateways=1 --burstPercent=50
```

---

## 6) Common command-line options

- `--nNodes` : number of end devices (default in example ~200)
- `--nGateways` : number of gateways (default 1)
- `--burstPercent` : percent of nodes that will enter burst mode (0-100)
- `--nChannels` : informational number of virtual channels (example uses this for configuration)

---

## Examples

- Small smoke-run:

```bash
export LD_LIBRARY_PATH=/home/satyamwsl/IoT/ns-3-dev/build/lib:$LD_LIBRARY_PATH
/home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation --nNodes=10 --nGateways=1 --burstPercent=30
```

- Larger experiment (100 nodes, 2 gateways, 60% burst):

```bash
export LD_LIBRARY_PATH=/home/satyamwsl/IoT/ns-3-dev/build/lib:$LD_LIBRARY_PATH
/home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation --nNodes=100 --nGateways=2 --burstPercent=60 --nChannels=8
```

---

## 7) If you see "Permission denied" when running the binary

- Ensure the binary is executable:

```bash
chmod +x /home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation
```

- If the linker failed previously with "Permission denied" (during build) and produced no binary, re-run the build. If linking fails trying to write to a root path like `//ns3...`, check your CMake cache and re-run the top-level `./ns3` wrapper.

---

## 8) Rebuild after code edits

- After modifying code under `scratch/burstmac/` or elsewhere, rebuild the project:

```bash
cd /home/satyamwsl/IoT/ns-3-dev
./ns3         # or: /usr/bin/cmake --build cmake-cache -j 5
```

---

## 9) Collecting results

- The example prints a LoraPacketTracker summary at the end (sent / received counts). Redirect stdout to a file to capture logs and metrics:

```bash
export LD_LIBRARY_PATH=/home/satyamwsl/IoT/ns-3-dev/build/lib:$LD_LIBRARY_PATH
/home/satyamwsl/IoT/ns-3-dev/build/burstmac-simulation --nNodes=200 --nGateways=1 --burstPercent=50 > run-output.txt 2>&1
```

---

## 10) Next steps and troubleshooting tips

- If the example crashes early: run a smaller scenario and enable `NS_LOG` for the LoRa components to see why.
- If linking errors mention unwritable output paths, inspect `cmake-cache/CMakeCache.txt` and the `scratch/burstmac/CMakeLists.txt` to ensure the outputs are directed to the repo `build/` directory.
- If you want me to run a smoke test here and paste the output, tell me the parameters to use and I'll run it.

---

## Contact / Support

- If you want an automated script to run multiple parameter sweeps and save results/plots, I can add `analysis/run_and_plot.py` (already present) and show how to use it.

-- End of file
