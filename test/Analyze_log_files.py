# Analyze_log_files.py
#
# 

import sys
import pandas as pd
import matplotlib.pyplot as plt
import msvcrt  # Windows keypress handling
#import termios, tty  # Linux/macOS keypress handling

def find_first_exceeding(series, threshold):
    """
    Finds and prints the index of the first item in a Pandas Series that exceeds the specified threshold.

    Parameters:
    - series (pd.Series): The Pandas Series to search.
    - threshold (float): The number to compare against.

    Returns:
    - int: The index of the first item exceeding the threshold, or -1 if none found.
    """
    exceeding_index = series[series > threshold].index.min()
    
    if pd.notna(exceeding_index):  # Check if a valid index is found
        #print(f"First value exceeding {threshold} is at index: {exceeding_index}")
        return exceeding_index
    else:
        #print(f"No value in the series exceeds {threshold}.")
        return -1  # Return -1 if no value exceeds the threshold
        
def calculate_mhw_energy(sh_r, vh_r, sh_temp, vh_temp, sh_pwm, vh_pwm, voltage):
    power_sum = 0
    
    for i in range(len(sh_pwm)):
        # Calculate temperature compensated R values
        sh_r_temp = sh_r*(1+(sh_temp[i]-25)*0.0039)
        vh_r_temp = vh_r*(1+(vh_temp[i]-25)*0.0039)
                
        power1 = voltage[i] * (voltage[i]/1e6) * (sh_pwm[i] / 255) / sh_r 
        power2 = voltage[i] * (voltage[i]/1e6) * (vh_pwm[i] / 255) / vh_r 
        power_sum += power1 + power2
        if (i > 1195) & (i < 1205):
            print(f"{i}: sh_r_temp: {sh_r_temp} vh_r_temp: {vh_r_temp} power: {power1 + power2}")
            
        power_avg = power_sum / len(sh_pwm)
        energy_mwh = power_avg *1000 * len(sh_pwm) / 3600
        
    print(f"power_sum: {power_sum}")
    print(f"calculate_mhw_energy: {energy_mwh}")

def print_logfile_summary(file_path):
 # Read all lines from the file
        with open(file_path, 'r', encoding='utf-8') as file:
            lines = file.readlines()
        
        print("\nfile_path:")
        for line in lines[:4]:
            print(line.strip())

        # Print the last 3 lines
        for line in lines[-3:]:
            print(line.strip())
       
def plot_data(time, title, y_label, dataset1, dataset1_label, dataset2 = [], dataset2_label = None):
    # Plot the data
    plt.figure(figsize=(10, 6))
    plt.plot(time, dataset1, label=dataset1_label, marker='o')
    if dataset2_label is not None:
        plt.plot(time, dataset2, label=dataset2_label, marker='s')

    # X-axis formatting: Show only 1 in 60 labels
    plt.xticks(time[::60], rotation=45)  # Rotate for better readability
    
    # Labels and title
    plt.xlabel("Time")
    plt.ylabel(y_label)
    plt.title(title)
    plt.legend()
    plt.grid(True)

    # Show the plot
    #plt.show()
    plt.show(block=False)
    input("Press Enter to continue...")
    plt.close()

def process_logfile(file_path, file_path2 = None):
    """Reads a CSV file, skips first 6 and last 4 lines, and plots column 1 vs. columns 2 and 5."""
    try:
        
        # Read CSV file while skipping the first 6 rows
        df = pd.read_csv(file_path, skiprows=6)
        # Drop last 4 rows
        df = df.iloc[:-4]

        # Ensure there are enough columns
        if df.shape[1] < 6:
            print("Error: The CSV file does not have enough columns (needs at least 6).")
            return

        time = df.iloc[:, 0]  # First column (time in msec)

        if file_path2 is not None:
            df2 = pd.read_csv(file_path2, skiprows=6)
            # Drop last 4 rows
            df2 = df2.iloc[:len(time)]

            # Ensure there are enough columns
            if df2.shape[1] < 6:
                print("Error: The CSV file does not have enough columns (needs at least 6).")
                return
            voltage2 = df2.iloc[:, 7]  # Fifth column (Voltage in mv)
            vh_temp2 = df2.iloc[:, 4]  # Fifth column (VH Temperature)
            vh_pwm2 = df2.iloc[:, 6]  #  Seventh column (VH PWM)


        # Extract relevant columns
        sh_temp = df.iloc[:, 1]  # Second column (SH Temperature)
        vh_temp = df.iloc[:, 4]  # Fifth column (VH Temperature)
        sh_pwm = df.iloc[:, 3]  # fourth column (SH PWM)
        vh_pwm = df.iloc[:, 6]  #  Seventh column (VH PWM)
        voltage = df.iloc[:, 7]  # Fifth column (Voltage in mv)

        sh_ramp_time = find_first_exceeding(sh_temp, 65) 
        print(f"sh_ramp_time: {sh_ramp_time}")

        sh_r = 4.27
        vh_r = 4.18
        calculate_mhw_energy(sh_r, vh_r, sh_temp, vh_temp, sh_pwm, vh_pwm, voltage)

        if file_path2 is None:
            plot_data(time, file_path, "Temperature (c)", sh_temp, "SH Temperature", vh_temp, "VH Temperature")
            plot_data(time, file_path, "PWM Control (0 to 255)", sh_pwm, "SH PWM", vh_pwm, "VH PWM")
            plot_data(time, file_path, "Battery Voltage", voltage, "VBat")
        else:
            plot_data(time, file_path, "Temperature (c)", vh_temp, "VH_Temp (Amazon)", vh_temp2, "VH_Temp2 (Kirkland)")
            plot_data(time, file_path, "VH PWM Control", vh_pwm, "VH PWM", vh_pwm2, "VH PWM2")
            plot_data(time, file_path, "Battery Voltage", voltage, "VBat (Amazon)", voltage2, "VBat (Kirkland)")

    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except pd.errors.EmptyDataError:
        print("Error: The file is empty or has insufficient data.")
    except Exception as e:
        print(f"Error: {e}")

def main():
    """Main function to handle command-line argument."""
    if len(sys.argv) < 2:
        print("Usage: python script.py <mk_log_file> [optional: mk_log_file2]")
        return
    elif len(sys.argv) == 2:
        file_path = sys.argv[1]
        process_logfile(file_path)
        print_logfile_summary(file_path)
    elif len(sys.argv) == 3:
        file_path = sys.argv[1]
        file_path2 = sys.argv[2]
        process_logfile(file_path, file_path2)
        print_logfile_summary(file_path)
        print_logfile_summary(file_path2)

if __name__ == "__main__":
    main()
