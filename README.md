# Distributed Resource Allocation & Fault-Tolerance Optimizer

## Overview

This project implements a heuristic-based system for allocating servers across racks and pools in a data center environment under failure constraints.

The objective is to maximize **guaranteed capacity (GP)** — the minimum capacity that remains available in each pool even if any single rack fails.

---

## Problem Setting

* A data center consists of multiple **racks**, each with fixed **slots**
* Some slots are **unavailable (dead)**
* A set of **servers** must be placed, each with:

  * size (number of slots required)
  * capacity (compute power)
* Servers must be assigned to **pools** for redundancy

---

## Objective

Maximize fault tolerance by optimizing:

**Guaranteed Capacity (GP):**
[
GP = \text{Total Capacity} - \text{Maximum Capacity in any one rack}
]

This ensures that if the worst rack fails, the system retains as much capacity as possible.

---

## Approach

The solution is divided into three stages:

### 1. Segment Extraction

* Convert each rack into contiguous free segments
* Each segment is represented as:

  ```
  (segment_length, start_index)
  ```

---

### 2. Server Placement (Greedy)

* Servers are sorted by:

  * higher capacity first
  * then larger size
* For each server:

  * find the smallest segment that fits (`lower_bound`)
  * choose rack minimizing:

    ```
    (current rack capacity, segment size)
    ```
* Place server and update segments

---

### 3. Pool Assignment (Global Heuristic)

* Servers are assigned to pools in descending capacity order
* For each assignment:

  * simulate placing server in each pool
  * compute:

    * new GP for that pool
    * minimum GP across all pools
* Choose pool that **maximizes the worst-case GP**

Tie-breaking:

* lower total pool load preferred
* higher individual GP preferred

---

## Key Features

* Efficient segment tracking using ordered sets
* Greedy placement with fragmentation awareness
* Global optimization for fault tolerance
* Handles arbitrary rack failures
* Deterministic and fast

---

## Complexity

* Segment construction: **O(R × S)**
* Placement: **O(M × R × log S)**
* Pool assignment: **O(M × P log P)**

Where:

* R = racks
* S = slots per rack
* M = servers
* P = pools

---

## Example Input Format

```
R S D P M
(rack, slot) × D
(server_length, capacity) × M
```

---

## Output Format

For each server:

```
rack_id slot_id pool_id
```

If a server cannot be placed:

```
x
```

---

## How to Run

```bash
g++ -std=c++20 main.cpp -O2 -o allocator
./allocator < input.txt > output.txt
```

---

## Design Insights

* Separates **placement** and **fault tolerance optimization**
* Uses **heuristics instead of exact optimization** (NP-hard problem)
* Balances:

  * space efficiency
  * load distribution
  * failure resilience

---

## Future Improvements

* Compare multiple heuristics (random, first-fit, etc.)
* Add visualization of rack layouts
* Simulate multiple simultaneous failures
* Parallelize placement for large-scale inputs

---

## Motivation

This project models real-world distributed system challenges:

* resource allocation under constraints
* fault tolerance
* load balancing

It is inspired by large-scale data center scheduling and optimization problems.
