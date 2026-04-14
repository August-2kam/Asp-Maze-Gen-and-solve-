import argparse
import sys

def generate_lp(rows, cols, start_x, start_y, end_x, end_y, filename="input.lp"):
    lp_content = f"""#const rows = {rows}.
#const cols = {cols}.

size(rows, cols).

start({start_x},{start_y}).
end({end_x},{end_y}).
"""

    # write to the file
    try:
        with open(filename, "w") as f:
            f.write(lp_content)
        print(f"Generated '{filename}'")
        print(f"Grid: {rows}x{cols} | Start: ({start_x}, {start_y}) | End: ({end_x}, {end_y})")
    except IOError as e:
        print(f"Error writing to file: {e}")
        sys.exit(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate an input.lp file for the maze generator.")
    
    # fefine command-line arguments with defaults
    parser.add_argument("-r", "--rows", type=int, default=25, help="Number of rows in the maze.")
    parser.add_argument("-c", "--cols", type=int, default=25, help="Number of columns in the maze.")

    parser.add_argument("-s", "--start", type=int, nargs=2, default=[1, 1], help="Start coordinates: X Y")
    parser.add_argument("-e", "--end", type=int, nargs=2, help="End coordinates: X Y. Defaults to (rows, cols).")
    
    parser.add_argument("-o", "--out", type=str, default="input.lp", help="Output filename.")

    args = parser.parse_args()

    # if no end coordinate is provided, default to the bottom-right corner
    final_end_x = args.end[0] if args.end else args.rows
    final_end_y = args.end[1] if args.end else args.cols

    # simple bounds checking
    if not (1 <= args.start[0] <= args.rows and 1 <= args.start[1] <= args.cols):
        print("Warning: Start coordinates are outside the grid boundaries.")
    if not (1 <= final_end_x <= args.rows and 1 <= final_end_y <= args.cols):
        print("Warning: End coordinates are outside the grid boundaries.")

    generate_lp(args.rows, args.cols, args.start[0], args.start[1], final_end_x, final_end_y, args.out)