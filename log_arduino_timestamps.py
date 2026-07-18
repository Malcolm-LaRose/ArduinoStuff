import serial
import time
import numpy as np


PORT = "COM3"
BAUD = 500000

ser = serial.Serial(PORT, BAUD, timeout=0.2)

time.sleep(3)

ser.reset_input_buffer()


filename_txt = "data.txt"

V_PER_COUNT = 7.8125e-6  # ADS1115 GAIN_SIXTEEN scale


with open(filename_txt, "w") as f:

    print("Logging... Ctrl+C to stop")

    printCount = 0
    start_time = time.perf_counter()
    end_time = start_time

    while True:

        try:

            # Read timestamp (4 bytes) + ADC (2 bytes)
            data = ser.read(6)

            if len(data) == 6:

                timestamp = np.frombuffer(
                    data[0:4],
                    dtype=np.uint32
                )[0]

                raw = np.frombuffer(
                    data[4:6],
                    dtype=np.int16
                )[0]


                voltage = raw * V_PER_COUNT


                if printCount % 8 == 0:
                    print(
                        f"{timestamp/1e6:.6f}s  {voltage:.5f} V"
                    )


                # timestamp, voltage
                f.write(
                    f"{timestamp/1e6:.6f}\t{voltage:.8f}\n"
                )


                end_time = time.perf_counter()
                printCount += 1


        except KeyboardInterrupt:

            print("\nStopped.")

            elapsed_time = end_time - start_time

            print(
                f"Elapsed time: {elapsed_time:.6f} seconds"
            )

            break


ser.close()


# Count samples afterward

with open(filename_txt) as f:
    sample_count = sum(1 for _ in f)


print(
    f"Saved {sample_count} samples "
    f"at {sample_count / elapsed_time:.3f} samples/sec"
)