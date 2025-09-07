#!/usr/bin/env python3
import argparse
import shutil
import socket
import subprocess
import sys
from urllib.request import Request, urlopen
from urllib.error import URLError, HTTPError


ENDPOINTS = [
    ("https://connectivitycheck.gstatic.com/generate_204", {204, 200}),
    ("https://www.google.com/generate_204", {204, 200}),
    ("https://1.1.1.1/cdn-cgi/trace", {200}),
]


def http_check(timeout: float = 3.0) -> bool:
    headers = {"User-Agent": "curl/7.85.0"}
    for url, ok_codes in ENDPOINTS:
        req = Request(url, headers=headers)
        try:
            with urlopen(req, timeout=timeout) as resp:
                if resp.status in ok_codes:
                    return True
        except HTTPError as e:
            if e.code in ok_codes:
                return True
        except URLError:
            pass
    return False


def dns_check(host: str = "nixos.org", timeout: float = 2.0) -> bool:
    # socket has no per-call timeout for getaddrinfo; set global default briefly
    old_to = socket.getdefaulttimeout()
    try:
        socket.setdefaulttimeout(timeout)
        socket.getaddrinfo(host, None)
        return True
    except Exception:
        return False
    finally:
        socket.setdefaulttimeout(old_to)


def ping_check(addr: str = "1.1.1.1", timeout: float = 2.0) -> bool:
    ping_bin = shutil.which("ping")
    if not ping_bin:
        return False
    # Linux ping uses -W (seconds) for timeout; -c 1 one packet
    try:
        proc = subprocess.run([ping_bin, "-c", "1", "-W", str(int(timeout)), addr],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return proc.returncode == 0
    except Exception:
        return False

def main():
    parser = argparse.ArgumentParser(description="Sjekk kun om internett fungerer og skriv grønn hake ved suksess.")
    parser.add_argument("--quiet", action="store_true", help="Ingen utskrift, kun exit-kode")
    parser.add_argument("--verbose", action="store_true", help="Skriv feildetaljer ved manglende tilgang")
    args = parser.parse_args()

    connected = http_check()
    # Supplementary signals (for verbose diagnostics only)
    limited = False
    dns_ok = False
    icmp_ok = False
    if not connected:
        dns_ok = dns_check()
        icmp_ok = ping_check()
        limited = dns_ok or icmp_ok

    # Output formatting
    is_tty = sys.stdout.isatty()
    GREEN = "\033[32m" if is_tty else ""
    RED = "\033[31m" if is_tty else ""
    RESET = "\033[0m" if is_tty else ""
    CHECK = "✔"
    CROSS = "✖"

    if not args.quiet:
        if connected:
            print(f"{GREEN}{CHECK}{RESET} Du har tilgang til internett.")
        else:
            msg = "Ingen internettilgang."
            if args.verbose:
                details = []
                details.append(f"HTTP={'OK' if connected else 'FAIL'}")
                details.append(f"DNS={'OK' if dns_ok else 'FAIL'}")
                details.append(f"PING={'OK' if icmp_ok else 'FAIL'}")
                if limited:
                    msg += " (Begrenset: " + ", ".join(details) + ")"
                else:
                    msg += " (" + ", ".join(details) + ")"
            print(f"{RED}{CROSS}{RESET} {msg}")

    # Exit code: 0 only when HTTP works
    sys.exit(0 if connected else 1)


if __name__ == "__main__":
    main()
