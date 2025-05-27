# K2SH - A Simple Unix Shell

**K2SH** is a lightweight, custom-built shell implemented in C. It supports basic command execution, built-in functions, and pipelines using `|`. I built this to learn how Unix-like shells work under the hood.

---

## 🚀 Features

- Run external commands (e.g., `ls`, `grep`, `cat`)
- Execute pipelines like `ls | grep txt`
- Built-in commands:
  - `cd` – Change directory
  - `pwd` – Print working directory
  - `echo` – Print text
  - `history` – Show command history
  - `help` – List built-in commands
  - `exit` – Exit the shell
- Tracks command history (up to 1000 entries)
- Modular design with a clean `Makefile` for building and running

---

## 🔧 Building K2SH

The project includes a flexible `Makefile` with debug and release modes.

### Build (Debug mode, default)

```bash
make
```

### Build (Release mode)
```bash
make release
```

---

## 🛠 Running K2SH
```bash
./k2sh
```
Or with `Makefile`
```bash
make run # Runs the debug version
make run-release # Runs the optimized version
```
