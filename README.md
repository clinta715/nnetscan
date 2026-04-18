# NNetScan

A Windows network scanner with both a GUI and command-line interface. Discovers hosts on a subnet using ARP and ICMP, resolves MAC addresses and vendors, and scans for open ports.

## Features

- **ARP scanning** — Discover hosts via ARP requests
- **ICMP ping** — Fallback host discovery via ICMP echo
- **Port scanning** — Checks common ports on discovered hosts
- **MAC vendor lookup** — Resolves OUI vendor from MAC address
- **GUI mode** — Win32 native interface with live results
- **CLI mode** — Headless scanning with table or JSON output

## Requirements

- Windows 7 or later
- Visual Studio 2019+ (MSVC) with Windows SDK
- CMake 3.15+ (optional, for CMake builds)

## Building

### Using PowerShell build script

```powershell
.\build.ps1
```

### Using CMake

```powershell
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Usage

### GUI Mode

Run with no arguments to launch the graphical interface:

```
nnetscan.exe
```

Select a network interface, configure scan options, and click "Start Scan".

### CLI Mode

Pass any command-line argument to enter CLI mode:

```
nnetscan.exe --subnet <CIDR> [options]
```

#### Options

| Flag | Description |
|------|-------------|
| `--subnet <CIDR>` | Subnet to scan in CIDR notation (e.g. `192.168.1.0/24`) |
| `--interface <idx>` | Interface index from `--list-interfaces` (default: auto-detect) |
| `--arp` | Enable ARP scan (default: on) |
| `--no-arp` | Disable ARP scan |
| `--icmp` | Enable ICMP ping (default: on) |
| `--no-icmp` | Disable ICMP ping |
| `--json` | Output results as JSON |
| `--list-interfaces` | List available network interfaces |
| `--help` | Show help message |

#### Examples

```bash
# List available interfaces
nnetscan.exe --list-interfaces

# Scan a subnet
nnetscan.exe --subnet 192.168.1.0/24

# ARP-only scan with JSON output
nnetscan.exe --subnet 10.0.0.0/24 --no-icmp --json

# Scan using a specific interface
nnetscan.exe --subnet 192.168.1.0/24 --interface 1
```
