import sys
import datetime
            
class FileReader:
    """Class to read and print contents of a file line by line."""
    
    def __init__(self, file_path):
        """Initialize with file path."""
        self.file_path = file_path
        self.logfile_open = False
        
    def open_logfile(self, suffix):
        if self.logfile_open:
            self.close_logfile()

        current_time = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        file_name = f"logs\{current_time}_{suffix}.log"
        print(f"\tOpening log file: {file_name}")
        self.log = open(file_name, "a")
        self.logfile_open = True
        
    def close_logfile(self):
        print(f"\tClosing log file")
        self.log.close()
        self.logfile_open = False        
        
    def process_line(self, line):
        print(line, end='')  # Print each line without adding extra newlines

        # open a new log file each time the MK board powers up
        keyword = "HW:"
        if keyword in line:
            boardnum = line.split(keyword, 1)[1].strip()
            print(f"\tFound: {boardnum}")
            self.open_logfile(boardnum)
            
        if self.logfile_open:
            self.log.write(line)
            
        # close the log file at the end of the amplification sequence
        keyword = "valve_ramp_time"
        if keyword in line:
            boardnum = line.split(keyword, 1)[1].strip()
            print(f"\tFound: {keyword}")
            self.close_logfile()
            
        

    def read_file(self):
        """Read the file and print each line."""
        try:
            with open(self.file_path, 'r') as file:
                for line in file:
                    self.process_line(line)
                    
        except FileNotFoundError:
            print(f"Error: The file '{self.file_path}' was not found.")
        except Exception as e:
            print(f"Error: {e}")


def main():
    """Main function to check command-line arguments and run the FileReader."""
    if len(sys.argv) != 2:
        print("Usage: python logger.py <filename>")
        return

    file_path = sys.argv[1]
    reader = FileReader(file_path)
    reader.read_file()

if __name__ == "__main__":
    main()
