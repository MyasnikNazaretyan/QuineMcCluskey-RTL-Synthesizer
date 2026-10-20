# Quine-McCluskey Logic Synthesizer & Verilog RTL Generator

A high-performance C++17 implementation of the **Quine-McCluskey tabular minimization algorithm** for Electronic Design Automation (EDA) and digital logic synthesis. 

Given a set of Boolean minterms, this engine computes all Prime Implicants using bitwise representation, extracts the minimal Sum-of-Products (SOP) cover via Essential Prime Implicant detection and a Greedy Set Cover heuristic, and automatically generates **synthesizable Verilog RTL**.

## Key Engineering Highlights

* **Zero-String Bitwise Architecture (`O(1)` Adjacency Check):** Instead of slow string comparisons (`"01-1"`), implicants are represented using pairs of 32-bit unsigned integers (`uint32_t value` and `uint32_t mask`). Hamming distance and single-bit adjacency checks are executed in constant time using bitwise XOR and power-of-two manipulation:
  ```cpp
  (mask == other.mask) && ((diff != 0) && ((diff & (diff - 1)) == 0))
  ```
* **Brian Kernighan's Bit-Counting Algorithm:** Minterms are partitioned into Hamming-weight buckets in $O(k)$ time (where $k$ is the number of set bits) using `temp &= (temp - 1)`.
* **Set Cover Optimization (Prime Implicant Chart):** Implements a two-stage reduction engine that first isolates **Essential Prime Implicants** (minterms covered by a single implicant) and subsequently resolves cyclic/remaining covers using a greedy maximum-coverage heuristic.
* **Automated Verilog RTL Codegen:** Translates the minimized Boolean cover directly into a synthesizable structural/dataflow Verilog module (`assign F = ...`), bridging the gap between algorithmic C++ optimization and FPGA hardware synthesis.
* **Modern C++17 Practices:** Utilizes move semantics (`std::move`) during iterative group transitions, `reserve()` pre-allocations to minimize heap fragmentation, and `const` reference data access.

---

## Project Structure

```text
qm-logic-synthesizer/
├── include/
│   ├── Implicant.hpp          # Bitwise implicant representation & bit-manipulation core
│   └── QuineMcCluskey.hpp     # Prime implicant generator, Set Cover solver & Verilog emitter
├── src/
│   └── main.cpp               # Synthesis driver & testbench example
└── README.md
```

---

## Build & Run Instructions

Requires a C++17 compliant compiler (`clang++` or `g++`).

```bash
# Clone the repository
git clone [https://github.com/](https://github.com/)<your-username>/qm-logic-synthesizer.git
cd qm-logic-synthesizer

# Compile with O3 optimization
clang++ -std=c++17 -O3 -Iinclude src/main.cpp -o synthesizer

# Run the synthesizer
./synthesizer
```

---

## Example Output

For the 4-variable Boolean function $F(A, B, C, D) = \sum m(0, 1, 2, 5, 6, 7, 8, 9, 10, 14)$:

```text
=== Generated Prime Implicants ===
0-01  -->  A'C'D
01-1  -->  A'BD
011-  -->  A'BC
-00-  -->  B'C'
-0-0  -->  B'D'
--10  -->  CD'

=== Minimized Boolean Expression (SOP) ===
F = B'C' + CD' + A'BD

// Auto-generated Synthesizable Verilog RTL
module minimized_logic(
    input A, B, C, D,
    output F
);
    assign F = (~B & ~C) | (C & ~D) | (~A & B & D);
endmodule
```

---

## Algorithmic Pipeline

1. **Hamming Weight Partitioning:** Initial minterms are grouped by their bit population count.
2. **Iterative Orthogonal Merging:** Adjacent groups $G_i$ and $G_{i+1}$ are compared. Terms differing by a single bit with identical dash masks are merged via `result.mask = mask | (val1 ^ val2)`. Unmerged terms are promoted to Prime Implicants.
3. **Prime Implicant Chart Resolution:**
   * Extracts *Essential Prime Implicants* by identifying minterms with unique coverage.
   * Applies a greedy heuristic to cover residual minterms with the minimum number of implicants.
4. **HDL Emission:** Emits valid Verilog-2001 continuous assignment statements.
