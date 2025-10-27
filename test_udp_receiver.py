#!/usr/bin/env python3
"""
UDP Test Receiver for Arduino Multi-Sensor System

This script listens for statsd-formatted UDP packets from the Arduino
and validates their format and content.

Usage:
    python3 test_udp_receiver.py [--port PORT] [--host HOST]

Default: Listens on 0.0.0.0:8125
"""

import socket
import argparse
import re
from datetime import datetime
from collections import defaultdict

# Expected statsd message format regex
STATSD_PATTERN = re.compile(
    r'^sensor\.([a-zA-Z_]+):([\d.]+)\|g\|#board_id:([A-F0-9]+),board_type:(\w+),sensor_type:(\w+)$'
)

class UDPTestReceiver:
    def __init__(self, host='0.0.0.0', port=8125):
        self.host = host
        self.port = port
        self.socket = None
        self.stats = defaultdict(int)
        self.measurements = defaultdict(list)

    def start(self):
        """Start the UDP listener"""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((self.host, self.port))

        print(f"========================================")
        print(f"UDP Test Receiver Started")
        print(f"Listening on {self.host}:{self.port}")
        print(f"Waiting for packets from Arduino...")
        print(f"Press Ctrl+C to stop")
        print(f"========================================\n")

        try:
            while True:
                data, addr = self.socket.recvfrom(1024)
                self.process_packet(data, addr)
        except KeyboardInterrupt:
            print("\n\nShutting down...")
            self.print_summary()
        finally:
            self.socket.close()

    def process_packet(self, data, addr):
        """Process received UDP packet"""
        try:
            message = data.decode('utf-8').strip()
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            # Validate message format
            match = STATSD_PATTERN.match(message)

            if match:
                measure_name = match.group(1)
                value = float(match.group(2))
                board_id = match.group(3)
                board_type = match.group(4)
                sensor_type = match.group(5)

                # Update statistics
                self.stats['total_packets'] += 1
                self.stats[f'measure_{measure_name}'] += 1
                self.measurements[measure_name].append(value)

                # Print formatted output
                print(f"[{timestamp}] ✓ VALID from {addr[0]}:{addr[1]}")
                print(f"  Measure: {measure_name} = {value}")
                print(f"  Board: {board_id} (type: {board_type})")
                print(f"  Sensor: {sensor_type}")
                print(f"  Raw: {message}")
                print()

                # Validate value ranges
                self.validate_measurement(measure_name, value)

            else:
                self.stats['invalid_packets'] += 1
                print(f"[{timestamp}] ✗ INVALID from {addr[0]}:{addr[1]}")
                print(f"  Raw: {message}")
                print(f"  Error: Does not match expected statsd format")
                print()

        except Exception as e:
            self.stats['error_packets'] += 1
            print(f"[{timestamp}] ✗ ERROR processing packet from {addr[0]}:{addr[1]}")
            print(f"  Error: {e}")
            print()

    def validate_measurement(self, name, value):
        """Validate measurement values are in reasonable ranges"""
        warnings = []

        # Define reasonable ranges
        ranges = {
            'temperature': (-40, 85),
            'humidity': (0, 100),
            'pressure': (800, 1200),
            'illuminance': (0, 120000),
            'uva': (0, 1000),
            'uvb': (0, 1000),
            'uvIndex': (0, 15),
            'AirQuality': (0, 1023)
        }

        if name in ranges:
            min_val, max_val = ranges[name]
            if value < min_val or value > max_val:
                warnings.append(f"⚠️  WARNING: {name} value {value} is outside typical range [{min_val}, {max_val}]")

        for warning in warnings:
            print(f"  {warning}")

    def print_summary(self):
        """Print statistics summary"""
        print("\n========================================")
        print("Session Summary")
        print("========================================")
        print(f"Total packets received: {self.stats['total_packets']}")
        print(f"Invalid packets: {self.stats.get('invalid_packets', 0)}")
        print(f"Error packets: {self.stats.get('error_packets', 0)}")
        print()

        print("Measurements received:")
        for measure_name, values in sorted(self.measurements.items()):
            count = len(values)
            if count > 0:
                avg = sum(values) / count
                min_val = min(values)
                max_val = max(values)
                print(f"  {measure_name:15s}: {count:3d} packets  (avg={avg:7.2f}, min={min_val:7.2f}, max={max_val:7.2f})")

        print("\n========================================")


def main():
    parser = argparse.ArgumentParser(
        description='UDP Test Receiver for Arduino Multi-Sensor System'
    )
    parser.add_argument(
        '--host',
        default='0.0.0.0',
        help='Host address to bind to (default: 0.0.0.0)'
    )
    parser.add_argument(
        '--port',
        type=int,
        default=8125,
        help='UDP port to listen on (default: 8125)'
    )

    args = parser.parse_args()

    receiver = UDPTestReceiver(host=args.host, port=args.port)
    receiver.start()


if __name__ == '__main__':
    main()
