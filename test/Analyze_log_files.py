# Analyze_log_files.py
#
# 

import sys
import pandas as pd
import matplotlib.pyplot as plt

def plot_csv_data(file_path):
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

        # Extract relevant columns
        x = df.iloc[:, 0]  # First column (time in msec)
        y_temp1 = df.iloc[:, 1]  # Second column (SH Temperature)
        y_temp2 = df.iloc[:, 4]  # Fifth column (VH Temperature)
        y_pwm1 = df.iloc[:, 3]  # fourth column (SH PWM)
        y_pwm2 = df.iloc[:, 6]  #  Seventh column (VH PWM)
        y_voltage = df.iloc[:, 7]  # Fifth column (Voltage in mv)

        # Plot the data
        plt.figure(figsize=(10, 6))
        plt.plot(x, y_temp1, label="SH Temperature", marker='o')
        plt.plot(x, y_temp2, label="VH Temperature", marker='s')

        # X-axis formatting: Show only 1 in 60 labels
        plt.xticks(x[::60], rotation=45)  # Rotate for better readability
        
        # Labels and title
        plt.xlabel("Time")
        plt.ylabel("Temperature (c)")
        plt.title(file_path)
        plt.legend()
        plt.grid(True)

        # Show the plot
        plt.show()

    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except pd.errors.EmptyDataError:
        print("Error: The file is empty or has insufficient data.")
    except Exception as e:
        print(f"Error: {e}")

def main():
    """Main function to handle command-line argument."""
    if len(sys.argv) != 2:
        print("Usage: python script.py <csv_file>")
        return

    file_path = sys.argv[1]
    plot_csv_data(file_path)

if __name__ == "__main__":
    main()
