import serial
import time
import numpy as np
import matplotlib.pyplot as plt


PORT = "COM3"
BAUD = 500000

ser = serial.Serial(PORT, BAUD, timeout=0.2)

time.sleep(3)

ser.reset_input_buffer()


filename_txt = "data.txt"

V_PER_COUNT = 7.8125e-6

DISPLAY_TIME = 20


timestamps = []
voltages = []


plt.ion()

fig, axs = plt.subplots(
    2, 2,
    figsize=(12, 8)
)

plt.show()


def calculate_fft(t, v):

    if len(v) < 16:
        return [], []

    dt = np.mean(np.diff(t))
    fs = 1 / dt

    x = v - np.mean(v)

    window = np.hanning(len(x))

    X = np.fft.rfft(x * window)

    freq = np.fft.rfftfreq(
        len(x),
        1/fs
    )

    return freq, np.abs(X)



def update_plots():

    t = np.array(timestamps)
    v = np.array(voltages)

    if len(t) < 20:
        return


    # Full time series

    axs[0,0].clear()
    axs[0,0].plot(
        t - t[0],
        v
    )
    axs[0,0].set_title("Full Time Series")
    axs[0,0].set_xlabel("Time (s)")
    axs[0,0].set_ylabel("Voltage (V)")


    # Full FFT

    freq, mag = calculate_fft(t, v)

    axs[0,1].clear()

    if len(freq):
        axs[0,1].plot(freq, mag)

    axs[0,1].set_title("Full FFT")
    axs[0,1].set_xlabel("Hz")


    # Last 20 seconds

    mask = t > t[-1] - DISPLAY_TIME

    ts = t[mask]
    vs = v[mask]


    axs[1,0].clear()
    axs[1,0].plot(
        ts - ts[0],
        vs
    )

    axs[1,0].set_title("Last 20 seconds")


    # 20 second FFT

    freq20, mag20 = calculate_fft(ts, vs)

    axs[1,1].clear()

    if len(freq20):
        axs[1,1].plot(freq20, mag20)

    axs[1,1].set_title("20 second FFT")


    plt.tight_layout()
    plt.draw()
    plt.pause(0.001)



with open(filename_txt, "w") as f:

    print("Logging... Ctrl+C to stop")

    last_plot = time.time()

    count = 0

    start_time = time.perf_counter()
    end_time = start_time


    try:

        while True:

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


                timestamps.append(t)
                voltages.append(voltage)


                f.write(
                    f"{t:.6f}\t{voltage:.8f}\n"
                )


                if count % 8 == 0:
                    print(
                        f"{t:.3f}s  {voltage:.6f} V"
                    )

                count += 1

                end_time = time.perf_counter()


            # Update display every 2 seconds

            if time.time() - last_plot > 2:

                update_plots()

                last_plot = time.time()


    except KeyboardInterrupt:

        print("\nStopped.")


ser.close()


elapsed = end_time - start_time

print(
    f"Saved {len(timestamps)} samples "
    f"at {len(timestamps)/elapsed:.3f} samples/sec"
)