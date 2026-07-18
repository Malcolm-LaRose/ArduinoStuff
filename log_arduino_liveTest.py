import serial
import time
import numpy as np
import matplotlib.pyplot as plt
import threading
import queue


PORT = "COM3"
BAUD = 500000

ser = serial.Serial(PORT, BAUD, timeout=0.2)

time.sleep(3)

ser.reset_input_buffer()


filename_txt = "data.txt"

V_PER_COUNT = 7.8125e-6

DISPLAY_TIME = 20


# Thread communication queue
data_queue = queue.Queue()


running = True


timestamps = []
voltages = []


def serial_reader():

    global running

    while running:

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


            t = timestamp / 1e6
            voltage = raw * V_PER_COUNT


            data_queue.put(
                (t, voltage)
            )



def calculate_fft(t, v):

    if len(v) < 16:
        return [], []

    dt = np.mean(np.diff(t))

    fs = 1 / dt


    x = v - np.mean(v)

    window = np.hanning(len(x))

    X = np.fft.rfft(
        x * window
    )

    freq = np.fft.rfftfreq(
        len(x),
        1/fs
    )

    return freq, np.abs(X)



def update_plots():

    if len(timestamps) < 20:
        return


    t = np.array(timestamps)
    v = np.array(voltages)


    # Full time series

    ax1.clear()

    ax1.plot(
        t - t[0],
        v
    )

    ax1.set_title("Full Time Series")
    ax1.set_xlabel("Time (s)")
    ax1.set_ylabel("Voltage (V)")


    # Full FFT

    ax2.clear()

    freq, mag = calculate_fft(t, v)

    if len(freq):
        ax2.plot(
            freq,
            mag
        )

    ax2.set_title("Full FFT")
    ax2.set_xlabel("Frequency (Hz)")


    # Last 20 seconds

    mask = t > t[-1] - DISPLAY_TIME

    ts = t[mask]
    vs = v[mask]


    ax3.clear()

    ax3.plot(
        ts - ts[0],
        vs
    )

    ax3.set_title("Last 20 seconds")


    # 20 second FFT

    ax4.clear()

    freq20, mag20 = calculate_fft(
        ts,
        vs
    )

    if len(freq20):
        ax4.plot(
            freq20,
            mag20
        )

    ax4.set_title("20 second FFT")


    plt.tight_layout()
    plt.draw()
    plt.pause(0.001)



fig, axes = plt.subplots(
    2,
    2,
    figsize=(12,8)
)

ax1 = axes[0,0]
ax2 = axes[0,1]
ax3 = axes[1,0]
ax4 = axes[1,1]


plt.ion()
plt.show()


# Start acquisition thread

thread = threading.Thread(
    target=serial_reader,
    daemon=True
)

thread.start()


print("Logging... Ctrl+C to stop")

printCount = 0

start_time = time.perf_counter()
last_plot = time.time()


try:

    with open(filename_txt, "w") as f:

        while True:


            # Consume all available samples

            while not data_queue.empty():

                t, voltage = data_queue.get()

                timestamps.append(t)
                voltages.append(voltage)
                
                printCount += 1

                if printCount % 8 == 0:
                    print(f"{t:.6f}s  {voltage:.6f} V")


                f.write(
                    f"{t:.6f}\t{voltage:.8f}\n"
                )


            # Update plots every 2 seconds

            if time.time() - last_plot >= 2:

                update_plots()

                last_plot = time.time()


            time.sleep(0.01)



except KeyboardInterrupt:

    print("\nStopped.")

    running = False


thread.join(timeout=1)

ser.close()


elapsed = time.perf_counter() - start_time

print(
    f"Saved {len(timestamps)} samples "
    f"at {len(timestamps)/elapsed:.3f} samples/sec"
)

plt.ioff()
plt.show()