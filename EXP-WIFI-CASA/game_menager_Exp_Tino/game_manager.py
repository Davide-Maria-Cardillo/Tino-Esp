import time
import socket
import sys
import csv
import random
import string
import threading
import os

# Generate a unique session ID
def generate_unique_id(length=8):
    return ''.join(random.choices(string.ascii_uppercase + string.digits, k=length))

# Create a new session ID
session_id = generate_unique_id()
csv_file_path = f"{session_id}_results.csv"

# UDP settings
LOCAL_UDP_IP = "192.168.0.100"
LOCAL_UDP_PORT = 40616
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((LOCAL_UDP_IP, LOCAL_UDP_PORT))

# IP addresses for ESP devices
ESP1_IP = "192.168.0.5"  # ESP1 with Button
ESP2_IP = "192.168.0.6"  # ESP2 with LED
ESP4_IP = "192.168.0.8"  # ESP4 with Wrong Button (Task 2)
ESP5_IP = "192.168.0.9"  # ESP5 with Wrong Button (Task 2)
ESP_PORT = 40616

# Condition Flags
qr_code_read = False
button_press_count = 0
counter_btn_wrg1 = 0  # Counter for wrong button press from ESP4
counter_btn_wrg2 = 0  # Counter for wrong button press from ESP5
last_button_press_time = 0  # Variable to store the last button press time

# Function to listen for UDP messages
def listen_for_udp():
    global button_press_count, counter_btn_wrg1, counter_btn_wrg2, qr_code_read, last_button_press_time
    while True:
        try:
            data, addr = sock.recvfrom(255)
            print(f"Received data from {addr}: {data}")  # Debugging line to see any received data
            received_message = data.decode('utf-8')
            print(f"Message received: {received_message}")  # Debugging line to see message content

            current_time = time.time()
            # Check if at least 1 second has passed since the last button press (debounce)
            if current_time - last_button_press_time >= 1:
                if addr[0] == ESP1_IP and received_message == "5":
                    # Handle message "5" from ESP1
                    if not qr_code_read:
                        button_press_count += 1
                        last_button_press_time = current_time
                        print(f"Button pressed on ESP1 while QR code not read. Count: {button_press_count}")
                        save_to_csv("Button Press Count (ESP1)", button_press_count, current_time)
                elif addr[0] == ESP4_IP and received_message == "6":
                    # Handle wrong button press from ESP4
                    counter_btn_wrg1 += 1
                    last_button_press_time = current_time
                    print(f"Wrong button pressed on ESP4. Count: {counter_btn_wrg1}")
                    save_to_csv("Wrong Button 1 Count (ESP4)", counter_btn_wrg1, current_time)
                elif addr[0] == ESP5_IP and received_message == "7":
                    # Handle wrong button press from ESP5
                    counter_btn_wrg2 += 1
                    last_button_press_time = current_time
                    print(f"Wrong button pressed on ESP5. Count: {counter_btn_wrg2}")
                    save_to_csv("Wrong Button 2 Count (ESP5)", counter_btn_wrg2, current_time)
                else:
                    print(f"Unhandled message from {addr[0]}: {received_message}")
        except socket.timeout:
            pass

# Save the cumulative count to CSV
def save_to_csv(counter_name, count, timestamp):
    file_exists = os.path.isfile(csv_file_path)
    with open(csv_file_path, mode="a", newline="") as file:
        writer = csv.writer(file)
        if not file_exists:
            writer.writerow(["Counter Name", "Count", "Timestamp"])
        writer.writerow([counter_name, count, timestamp])

# Function to write final counts to CSV with formatting
def write_final_counts():
    with open(csv_file_path, mode="a", newline="") as file:
        writer = csv.writer(file)
        writer.writerow([])  # Empty line to separate final counts
        writer.writerow(["   Final Button Press Count (ESP1)", button_press_count])
        writer.writerow(["   Final Wrong Button 1 Count (ESP4)", counter_btn_wrg1])
        writer.writerow(["   Final Wrong Button 2 Count (ESP5)", counter_btn_wrg2])

# Start a thread to listen for UDP messages
listener_thread = threading.Thread(target=listen_for_udp, daemon=True)
listener_thread.start()

# Main loop for user input
running = True

print("Starting UDP listener and user input...")
while running:
    # Check for keyboard input
    input_text = input("Type command (or press 'q' to quit): ")
    if input_text.lower() == 'q':
        # Write final counts with formatting
        write_final_counts()
        running = False
    elif input_text == "leggi QR code":
        qr_code_read = True
        # Send message "1" to ESP2 to turn LED green
        message = "1"
        sock.sendto(message.encode('utf-8'), (ESP2_IP, ESP_PORT))
        print(f"Sent message '{message}' to ESP2 at {ESP2_IP}")
