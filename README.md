# Operating Systems Assignment 4: File Systems and Disk Scheduling

**Course:** Operating Systems (CSA04)  
**Student Name:** [Student Name]  
**Register Number:** [Register Number]  
**Compiler:** GCC 13 (Ubuntu 13.2.0)  
**Linux Distribution:** Ubuntu 22.04 LTS  
**Compilation Standard:** `gcc -std=c11 -Wall -Wextra -o <output> <source>.c`

---

## 1. Directory Structure

```text
CSA04-Assignment4/
├── README.md
├── Task02/
│   └── file_alloc.c          # Task 2: Contiguous and Indexed file allocation
└── Task04/
    ├── disk_sched.c          # Task 4.1: Core disk scheduling (FCFS, SSTF, SCAN, C-SCAN)
    ├── disk_sched_sim.c      # Task 4.2: 10,000-request simulation under 3 load levels
    └── sstf_starvation.c     # Task 4.3: Demonstration of SSTF starvation and aging mitigation
```

---

## 2. Compilation Commands

All programs are written in strict ISO C11 and compile cleanly without warnings using `-std=c11 -Wall -Wextra`:

```bash
# 1. Compile Task 2: File Allocation
gcc -std=c11 -Wall -Wextra -o Task02/file_alloc Task02/file_alloc.c

# 2. Compile Task 4.1: Disk Scheduling Algorithms
gcc -std=c11 -Wall -Wextra -o Task04/disk_sched Task04/disk_sched.c

# 3. Compile Task 4.2: 10,000 Request Disk Scheduling Simulation
gcc -std=c11 -Wall -Wextra -o Task04/disk_sched_sim Task04/disk_sched_sim.c

# 4. Compile Task 4.3: SSTF Starvation Demonstration
gcc -std=c11 -Wall -Wextra -o Task04/sstf_starvation Task04/sstf_starvation.c
```

---

## 3. Execution Steps and Expected Outputs

### Task 2: File Allocation (`file_alloc.c`)

Demonstrates contiguous allocation (first-fit) and indexed allocation on a simulated 32-block disk. Files A (6 blocks), B (8 blocks), C (5 blocks), and D (7 blocks) are allocated; files B and D are released to induce fragmentation; then file E (10 blocks) attempts both allocation strategies.

#### Steps to Run:
```bash
./Task02/file_alloc
```

#### Expected Output:
```text
AAAAAA........CCCCC.............
E (10 blocks) contiguous: start -1
E (10 blocks) indexed:    index block 6, data blocks 7 8 9 10 11 12 13 19 20 21 
AAAAAAeEEEEEEECCCCC...EEE.......
```

#### Short Note on Results:
- **Contiguous Allocation:** Fails (returns `-1`) despite 21 total free blocks remaining on the disk. The free blocks are fragmented into three non-contiguous segments of size 8, 7, and 6 blocks, none of which can satisfy the contiguous 10-block requirement. This directly illustrates **external fragmentation**.
- **Indexed Allocation:** Successfully allocates File E by assigning block 6 as the dedicated index block (represented by lowercase `'e'`) and linking 10 scattered free blocks (blocks 7–13 and 19–21). Indexed allocation completely avoids external fragmentation.

---

### Task 4.1: Disk Scheduling Algorithms (`disk_sched.c`)

Implements and evaluates FCFS, SSTF, SCAN, and C-SCAN on an 8-request workload (`{98, 183, 37, 122, 14, 124, 65, 67}`) with initial head position at cylinder 53 on a 200-cylinder disk (0–199).

#### Steps to Run:
```bash
./Task04/disk_sched
```

#### Expected Output:
```text
FCFS   53 98 183 37 122 14 124 65 67 -> 640 cylinders
SSTF   53 65 67 37 14 98 122 124 183 -> 236 cylinders
SCAN   53 37 14 0 65 67 98 122 124 183 -> 236 cylinders
C-SCAN 53 65 67 98 122 124 183 199 , jump to 0 14 37 -> 382 cylinders (+199 for the jump)
```

#### Short Note on Results:
- **FCFS:** Traverses 640 cylinders due to wild back-and-forth oscillations between high and low cylinders.
- **SSTF:** Greedily serves the closest pending cylinder, dramatically cutting head travel to 236 cylinders.
- **SCAN:** Sweeps towards cylinder 0 first (servicing 37, 14, and hitting 0), then reverses outward to service cylinders up to 183, completing in 236 cylinders without starvation risk.
- **C-SCAN:** Sweeps upward to cylinder 199, jumps back to cylinder 0 (199-cylinder reset), and serves lower cylinders, totaling 382 cylinders with uniform wait time distribution.

---

### Task 4.2: Disk Scheduling Simulation (`disk_sched_sim.c`)

Simulates FCFS, SSTF, SCAN, C-SCAN, LOOK, and C-LOOK across 10,000 pseudo-random requests (cylinders 0–199) under three distinct system loads:
1. **Light Load:** 10 pending requests in queue
2. **Medium Load:** 50 pending requests in queue
3. **Heavy Load:** 200 pending requests in queue

Measures both average head movement per request (cylinders) and maximum waiting time (time units).

#### Steps to Run:
```bash
./Task04/disk_sched_sim
```

#### Expected Output:
```text
========================================================================================
        DISK SCHEDULING SIMULATION (Task 4.2): 10,000 REQUESTS (0-199)
        Initial Head Position: 53 | Cylinders: 200 | Workload: Uniform Random
========================================================================================

--- Light Load  (10 pending requests) ---
+----------+-------------------+--------------------+------------------+-----------------+
| Alg      | Total Head Travel | Avg Head Movement  | Max Waiting Time | Avg Wait Time   |
+----------+-------------------+--------------------+------------------+-----------------+
| FCFS     |            663084 |          66.31 cyl |          1346 ms |        662.9 ms |
| SSTF     |            130335 |          13.03 cyl |           856 ms |        130.2 ms |
| SCAN     |            132338 |          13.23 cyl |           391 ms |        132.2 ms |
| C-SCAN   |            198642 |          19.86 cyl |           397 ms |        198.3 ms |
| LOOK     |            124190 |          12.42 cyl |           375 ms |        124.1 ms |
| C-LOOK   |            180556 |          18.06 cyl |           395 ms |        180.4 ms |
+----------+-------------------+--------------------+------------------+-----------------+

--- Medium Load (50 pending requests) ---
+----------+-------------------+--------------------+------------------+-----------------+
| Alg      | Total Head Travel | Avg Head Movement  | Max Waiting Time | Avg Wait Time   |
+----------+-------------------+--------------------+------------------+-----------------+
| FCFS     |            663084 |          66.31 cyl |          4650 ms |       3308.3 ms |
| SSTF     |             27771 |           2.78 cyl |           758 ms |        138.4 ms |
| SCAN     |             26644 |           2.66 cyl |           394 ms |        132.5 ms |
| C-SCAN   |             39818 |           3.98 cyl |           397 ms |        197.4 ms |
| LOOK     |             26276 |           2.63 cyl |           392 ms |        131.1 ms |
| C-LOOK   |             39074 |           3.91 cyl |           397 ms |        194.4 ms |
+----------+-------------------+--------------------+------------------+-----------------+

--- Heavy Load  (200 pending requests) ---
+----------+-------------------+--------------------+------------------+-----------------+
| Alg      | Total Head Travel | Avg Head Movement  | Max Waiting Time | Avg Wait Time   |
+----------+-------------------+--------------------+------------------+-----------------+
| FCFS     |            663084 |          66.31 cyl |         15106 ms |      13137.9 ms |
| SSTF     |              6667 |           0.67 cyl |           381 ms |        132.6 ms |
| SCAN     |              6671 |           0.67 cyl |           378 ms |        132.3 ms |
| C-SCAN   |              9999 |           1.00 cyl |           397 ms |        196.6 ms |
| LOOK     |              6667 |           0.67 cyl |           378 ms |        132.2 ms |
| C-LOOK   |              9998 |           1.00 cyl |           397 ms |        196.1 ms |
+----------+-------------------+--------------------+------------------+-----------------+

========================================================================================
                      COMPREHENSIVE PERFORMANCE COMPARISON MATRIX
========================================================================================
+----------+-------------------------+-------------------------+-------------------------+
|          |  Light Load (10 pend)   |  Medium Load (50 pend)  |  Heavy Load (200 pend)  |
|Algorithm | Avg Mov (cyl)  Max Wait | Avg Mov (cyl)  Max Wait | Avg Mov (cyl)  Max Wait |
+----------+-------------------------+-------------------------+-------------------------+
| FCFS     |      66.31        1346 |      66.31        4650 |      66.31       15106 |
| SSTF     |      13.03         856 |       2.78         758 |       0.67         381 |
| SCAN     |      13.23         391 |       2.66         394 |       0.67         378 |
| C-SCAN   |      19.86         397 |       3.98         397 |       1.00         397 |
| LOOK     |      12.42         375 |       2.63         392 |       0.67         378 |
| C-LOOK   |      18.06         395 |       3.91         397 |       1.00         397 |
+----------+-------------------------+-------------------------+-------------------------+
```

#### Short Note on Results:
- **FCFS:** Produces an invariant average seek distance of 66.31 cylinders across all loads, perfectly matching theoretical expected value $(199 - 0) / 3 \approx 66.33$. As queue load increases, maximum wait time explodes to 15,106 ms.
- **SSTF:** Seek distance improves dramatically from 13.03 cylinders (light) to 0.67 cylinders (heavy). However, maximum wait time is significantly higher and more erratic than SCAN/LOOK due to local cluster favoritism.
- **SCAN & LOOK:** Consistently match SSTF seek efficiency while providing strictly bounded maximum wait times ($\approx 2 \times \text{disk diameter} \approx 390$ ms). LOOK outperforms SCAN slightly under light load because it avoids unnecessary trips to cylinders 0 and 199.
- **C-SCAN & C-LOOK:** Incur $\approx 50\%$ higher seek movement due to the unserviced return jump, but deliver the most uniform and predictable wait time distribution across all cylinder addresses.

---

### Task 4.3: SSTF Starvation Demonstration (`sstf_starvation.c`)

Demonstrates head starvation under SSTF when an initial request sits at cylinder 183 while new requests arrive continuously near cylinder 60 (cylinders 55–65) every millisecond. Also demonstrates how an aging policy guarantees timely servicing.

#### Steps to Run:
```bash
./Task04/sstf_starvation
```

#### Expected Output:
```text
########################################################################################
#                    DEMONSTRATION OF DISK HEAD STARVATION IN SSTF                     #
########################################################################################

========================================================================================
                  EXPERIMENT 1: PURE SSTF DISK SCHEDULING (STARVATION)
========================================================================================
Configuration:
 - Initial Head Position : 53
 - Starving Target       : Cylinder 183 (Arrived at t = 0 ms)
 - Workload Stream       : New requests near cylinder 60 [55-65] arriving every 1 ms
 - Total Simulation Time : 1000 ms

+----------+------------------------+------------+--------------------------------------+
| Time(ms) | Local Reqs Serviced    | Head Pos   | Cylinder 183 Status                  |
+----------+------------------------+------------+--------------------------------------+
|       10 |                     10 |         63 | STARVING (Waited   10 ms, dist=120)  |
|       50 |                     50 |         56 | STARVING (Waited   50 ms, dist=127)  |
|      100 |                    100 |         61 | STARVING (Waited  100 ms, dist=122)  |
|      250 |                    250 |         65 | STARVING (Waited  250 ms, dist=118)  |
|      500 |                    500 |         63 | STARVING (Waited  500 ms, dist=120)  |
|     1000 |                   1000 |         64 | STARVING (Waited 1000 ms, dist=119)  |
+----------+------------------------+------------+--------------------------------------+

RESULT: Cylinder 183 suffered COMPLETE STARVATION!
 - Serviced 1000 local requests near cylinder 60 while Request 183 remained unserviced.
 - Reason: SSTF is a greedy local optimizer. Because incoming local requests are
   within 55-65 cylinders of the head, distance to local requests (~0-12 cyl) is always
   substantially smaller than distance to cylinder 183 (~119 cyl).

========================================================================================
                  EXPERIMENT 2: SSTF WITH AGING (STARVATION MITIGATION)
========================================================================================
Aging Heuristic: Priority Score = abs(cylinder - head) - (alpha * waiting_time)
Parameters     : alpha = 0.50 (older requests receive progressively higher priority)

RESULT: Starvation Resolved! Request 183 was successfully serviced at t = 227 ms.
 - Local requests serviced before Request 183: 226
 - Waiting time for Request 183: 227 ms
 - Aging successfully bounded maximum latency by elevating the starving request's
   priority over newly arriving local requests.

========================================================================================
                                   SUMMARY & CONCLUSION                                 
========================================================================================
1. Vulnerability: Pure Shortest Seek Time First (SSTF) guarantees minimum average seek
   time locally, but is prone to starvation under heavy or localized request patterns.
2. Root Cause: Requests at the peripheral tracks (like cylinder 183) can be delayed
   indefinitely if traffic continuously feeds near the current cylinder head (~60).
3. Practical Solutions:
   - Elevator Algorithms (SCAN, C-SCAN, LOOK): Sweep across tracks sequentially, ensuring
     every cylinder is serviced within a bounded cycle time.
   - Aging / Deadline Schedulers: As implemented in production operating systems
     (e.g., Linux BFQ and Deadline schedulers), expiry timers guarantee service bounds.
```

#### Short Note on Results:
- In pure SSTF, Request 183 was starved for the entire 1,000 ms duration while 1,000 localized requests were processed. Because the seek distance to new local arrivals (0–12 cylinders) was always far shorter than the seek to cylinder 183 ($\sim 120$ cylinders), SSTF never ventured to cylinder 183.
- With Aging ($\alpha = 0.50$), the effective priority penalty of waiting time accumulates, eventually overriding the spatial distance penalty and servicing Request 183 at $t = 227\text{ ms}$. This mirrors modern OS I/O schedulers (such as Linux Deadline and BFQ).

---
