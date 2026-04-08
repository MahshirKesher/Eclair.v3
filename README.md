## 1.1. System Architecture (MVC Pattern)

The editor enforces a strict Model-View-Controller separation to decouple memory routing from raw terminal output.

**The Model (Data & State)**
* **`TextBuffer`:** The core memory engine. Implements the Piece Table architecture to ensure mathematically safe, non-destructive text mutations (insertions/deletions) without shifting physical bytes in RAM.
* **`Cursor`:** Maintains the strict logical 1D/2D coordinates of the active editing position, fully independent of terminal window boundaries.

**The View (Presentation)**
* **`Terminal`:** The low-level OS wrapper. Handles POSIX signal trapping, standard I/O, and raw/cooked mode toggling.
* **`Viewport`:** The spatial calculator. Maintains the mathematical scroll offset and boundary limits of the currently visible screen matrix.
* **`Renderer`:** The visual translator. Maps the logical `TextBuffer` state to the physical `Terminal` output, executing non-destructive visual formatting (e.g., suppressing `\r` carriage returns).

**The Controller (Routing & Orchestration)**
* **`EditorCore`:** The central orchestrator. Owns the main execution loop, bridges the Model and View through strict interfaces, and executes commands without ever parsing or mutating raw struct data directly.
* **`InputHandler`:** The hardware bridge. Detects raw byte streams from the keyboard and translates ANSI escape sequences into defined, actionable command structures.
* **`FileHandler`:** The OS disk bridge. Opens streams and routes memory chunks directly from the Model to the disk lock, ensuring file serialization executes with strictly O(1) auxiliary memory overhead.

## 2.1. Architectural Memory Constraints & RAM Footprint

* **Append-Only Heap Allocation:** The core `TextBuffer` enforces a strict Piece Table architecture. All document mutations (insertions/deletions) are non-destructive to the original loaded file buffer. New keystrokes are written to an append-only `std::string` buffer. This guarantees that heap allocations only scale with the user's active typing volume, never with the size of the underlying file.
* **Zero-Copy Traversal:** The document state is maintained via an array of lightweight `Piece` structs. Rendering and line-wrapping are achieved through `O(1)` offset calculations. The physical characters are never duplicated or shifted in RAM during an edit, completely preventing memory thrashing.
* **O(1) Auxiliary Memory on Save:** The `FileHandler` disk bridge is strictly decoupled from the memory model. During a save operation, the system does not concatenate the Piece Table into a contiguous string. Instead, it streams fragments directly from the disjointed buffers into an active `std::fstream`. This guarantees `O(1)` auxiliary RAM overhead during disk writes, mathematically preventing `std::bad_alloc` crashes caused by heap fragmentation or contiguous memory limits when handling massive files.

## 2.2. Execution Determinism & Time Complexity

The system is designed to guarantee deterministic response times for critical UI thread operations, isolating heavy O(N) workloads to strictly necessary initialization and termination phases. 

* **Initialization (Read & Map): `O(N)` time.** Where `N` is the total character count of the file. The initial load is strictly bound by disk I/O and the single-pass linear scan required to map newline byte-offsets for the rendering coordinate system.
* **Document Mutation (Insert/Delete): `O(P)` time.** Where `P` is the total number of active pieces in the table. Unlike naive string arrays that suffer from `O(N)` memory shifting on every keystroke, the Piece Table handles mid-document insertions by simply splitting a `Piece` struct and updating the array. Because `P` remains exponentially smaller than `N`, keystroke latency remains effectively constant regardless of file size.
* **Cursor Navigation (Logical to Global): `O(P)` time.** Translating a visual 2D terminal coordinate into a 1D global memory offset requires iterating through the piece descriptors to sum their lengths. 
* **Viewport Rendering: `O(V)` time.** Where `V` is the character capacity of the terminal viewport (Rows × Columns). The rendering engine is mathematically decoupled from the total file size (`N`). Drawing the screen requires extracting and rendering only the exact bytes currently visible to the user, preventing CPU spikes in massive documents.
* **Serialization (Save to Disk): `O(N)` time.** File saving requires sequential traversal of the Piece Table and sequential disk writing of all `N` characters. This is heavily bottlenecked by OS-level I/O, but RAM utilization remains strictly `O(1)` auxiliary space.

## 2.3. Build Chain & Reproducibility

This project enforces strict compiler adherence and a zero-tolerance warning policy to guarantee memory safety and execution stability.

* **Compiler:** GCC (GNU Compiler Collection)
* **Minimum required standard:** C++20. C++23 used for compilation.
* **Compilation flags:** `-Wall -Wextra -Werror`
  * These flags are mandated to ensure all structural warnings are treated as fatal build errors. The codebase is guaranteed to compile cleanly without silencing compiler outputs.
* **Build target:** `v3`

**Build Instructions:**
To compile the editor, execute the following command in the root directory:
```bash
$ make v3
```

## 2.4. System Boundaries & Known Limitations

The architecture intentionally omits certain modern text-editing guardrails to maintain strict execution determinism and a minimal codebase. 

* **Strict ASCII Encoding Constraint:** The memory-to-visual mapping algorithm strictly assumes a 1-to-1 correlation between bytes and terminal columns. Multibyte encodings (e.g., UTF-8) are unsupported. Attempting to load or insert multibyte characters will cause visual desynchronization between the logical cursor offset and the terminal rendering matrix.
* **Line-Ending Preservation:** The Model operates in strict binary mode (`std::ios::binary`). It does not dynamically convert `\r\n` (Windows) to `\n` (UNIX). The exact byte sequence loaded from disk is preserved in the Piece Table. To prevent raw-terminal rendering artifacts, carriage returns (`\r`) are suppressed visually in the View layer via non-destructive injection, guaranteeing zero-mutation saves across different operating systems.
* **Unmonitored Concurrency:** The `FileHandler` does not implement OS-level file locking or timestamp polling. The system assumes exclusive user ownership of the target file. External modifications made to the file on disk while the editor is running will be silently overwritten upon the next save execution.
* **Append Buffer Memory Ceiling:** While the Piece Table supports opening theoretically massive files in `O(N)` time, the append-only buffer used for capturing new mutations relies on a contiguous `std::string`. Therefore, the total volume of new character insertions within a single runtime session is bound by the host system's contiguous heap availability.
