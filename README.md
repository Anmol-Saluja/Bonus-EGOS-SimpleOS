# 🚀 SimpleOS – Lightweight OS with MLFQ Scheduler & Custom Shell

## 📌 Overview

SimpleOS is a lightweight educational operating system built on top of EGOS-2000.
This project explores both user-space and kernel-space programming by implementing:

* Custom shell utilities (`grep`, `wcl`)
* A Multi-Level Feedback Queue (MLFQ) scheduler

The goal is to understand how operating systems manage processes, scheduling, and low-level file interactions.

---

## 🧠 Features

### 🔹 Custom Shell Commands

#### `grep`

* Searches for a pattern in a file
* Prints matching lines

#### `wcl`

* Counts total number of lines across one or more files

👉 Implemented using low-level system calls (`dir_lookup`, `file_read`) without standard C file APIs.

---

### 🔹 MLFQ Scheduler

Implemented a preemptive Multi-Level Feedback Queue scheduler with:

* 5 priority levels (0 → 4)
* Dynamic priority adjustment based on CPU usage

**Time slices:**

* Level i → (i + 1) × 100 ms

**Priority reset:**

* Every ~10 seconds
* On shell input

---

### 🔹 Process Lifecycle Tracking

Each process tracks:

* ⏱ Turnaround Time
* ⚡ Response Time
* 🧮 CPU Time
* 🔁 Number of Timer Interrupts

All stats are printed on process termination.

---

## ⚙️ Tech Stack

* **Language:** C
* **Platform:** RISC-V
* **Emulator:** QEMU
* **Toolchain:** RISC-V GCC
* **Build System:** GNU Make

---

## ⚡ Implementation Highlights

### 🧩 Shell Commands

* Read files block-by-block using `file_read`
* Manual parsing of lines
* Pattern matching using `string.h`

### 🧩 Scheduler

Implemented in:

* `proc_yield()`

* `mlfq_update_level()`

* `mlfq_reset_level()`

* Priority-based scheduling (lower level = higher priority)

### 🧩 Interrupt Handling

Timer interrupts trigger:

* Context switch
* CPU time tracking
* Queue updates

---
