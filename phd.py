import time
import win32pdh

def get_real_time_cpu_frequency():
    query_handle = win32pdh.OpenQuery()

    # Add counters for % Processor Performance and Processor Frequency
    counter_perf = win32pdh.AddCounter(query_handle, r"\Processor Information(_Total)\% Processor Performance")
    counter_freq = win32pdh.AddCounter(query_handle, r"\Processor Information(_Total)\Processor Frequency")

    try:
        # Collect multiple samples before reading
        for _ in range(3):  
            win32pdh.CollectQueryData(query_handle)
            time.sleep(1)  # Allow the counters to stabilize

        while True:
            win32pdh.CollectQueryData(query_handle)
            time.sleep(1)

            # Get % Processor Performance
            _, perf_percentage = win32pdh.GetFormattedCounterValue(counter_perf, win32pdh.PDH_FMT_LONG)

            # Get Processor Frequency (Rated/Base Frequency)
            _, base_frequency = win32pdh.GetFormattedCounterValue(counter_freq, win32pdh.PDH_FMT_LONG)

            # Calculate real-time frequency
            real_time_frequency = (perf_percentage / 100) * base_frequency
            
            print(f"Estimated Real-Time CPU Frequency: {real_time_frequency:.2f} MHz")
    
    except KeyboardInterrupt:
        print("\nStopping monitoring.")
    
    finally:
        win32pdh.CloseQuery(query_handle)

get_real_time_cpu_frequency()
