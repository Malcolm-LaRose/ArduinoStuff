import serial
import time
import numpy as np

PORT = "COM3"
BAUD = 500000

ser = serial.Serial(PORT, BAUD, timeout=0.1)
time.sleep(3)

ser.reset_input_buffer()

filename_txt = "data.txt"

V_PER_COUNT = 7.8125e-6  # ADS1115 GAIN_SIXTEEN scale

with open(filename_txt, "w") as f:

    print("Logging... Ctrl+C to stop")
    printCount = 0
    start_time = time.perf_counter()
    while True:
        try:
            data = ser.read(2)

            if len(data) == 2:
                raw = np.frombuffer(data, dtype=np.int16)[0]
                voltage = raw * V_PER_COUNT
            
                if printCount % 4 == 0:
                    print(f"{voltage:.5f}")

                f.write(f"{voltage:.5f}\n")
                end_time = time.perf_counter()
                printCount += 1
                # f.flush()   # Leave this commented unless you need immediate disk writes

        except KeyboardInterrupt:
            print("\nStopped.")
            elapsed_time = end_time - start_time
            print(f"Elapsed time: {elapsed_time:.6f} seconds")
            break

ser.close()

# Count samples afterward
with open(filename_txt) as f:
    sample_count = sum(1 for _ in f)

print(f"Saved {sample_count} samples to {filename_txt} at {sample_count / elapsed_time:.3f} samples/sec")