# magicport
MagicPort is a lightweight, multi-threaded port scanner written in C. It efficiently scans a range of ports on a target host to identify open ports. The scanner uses threading to speed up the scanning process, making it a powerful tool for quick network reconnaissance.

## Features

   - Multi-threaded scanning: Scans ports concurrently using multiple threads.
   - Custom timeout: Allows users to specify the timeout for connection attempts.
   - Real-time results: Outputs open ports in real-time as they are discovered.
   - Cross-platform: Works on any Unix-like system that supports POSIX threads.

## Prerequisites

Before compiling and running MagicPort, make sure you have the following:

   - A C compiler (gcc recommended)
   - POSIX-compliant system (Linux, macOS, etc.)

Installation

   - Clone the repository:
```
git clone https://github.com/yourusername/magicport.git
cd magicport
```
  - Compile the source code:
```
gcc -o magicport magicport.c -lpthread
```
  - Run the scanner:
```
./magicport <hostname> <timeout>
```
  - <hostname>: The target IP address or domain name.
  - <timeout>: Connection timeout in seconds for each port.

## Example Usage

To scan ports on the target 192.168.1.1 with a 2-second timeout:

```bash
./magicport 192.168.1.1 2
```

The output will display the open ports found on the target host:

```
Scanning 192.168.1.1
--> Created 10 threads.
Port 22 is OPEN
Port 80 is OPEN
Port 443 is OPEN
```

## How It Works

MagicPort uses POSIX threads to scan multiple ports simultaneously. Each thread attempts to connect to a designated port using non-blocking sockets. If a connection is successful within the specified timeout, the port is reported as open.

## Code Overview

  - main: The entry point, responsible for setting up threads and starting the scan.
  - worker: The function executed by each thread, which calls the scanner function.
  - scanner: The core function that checks whether a specific port is open.

## License

This project is licensed under the MIT License. See the LICENSE file for details.
