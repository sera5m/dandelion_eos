# Function to generate and save sine, cosine, and tangent values for each radian angle (0 to 2π range in degrees)
import math
import csv

def generate_trig_csv_radians():
    sin_values = []
    cos_values = []
    tan_values = []
    radians_list = []

    # Iterate through degrees 0 to 365
    for degree in range(366):  # Including 365 to match the request (0-365 degrees)
        # Convert degree to radians
        radian = math.radians(degree)
        
        # Calculate sin, cos, and tan values in radians
        sin_val = math.sin(radian)
        cos_val = math.cos(radian)
        
        # Handle undefined tangent (set to 'inf' or similar)
        try:
            tan_val = math.tan(radian)
        except ValueError:
            tan_val = float('inf')  # Handle undefined tangent as infinity
        
        # Append radian and trig values to respective lists
        radians_list.append(radian)
        sin_values.append(sin_val)
        cos_values.append(cos_val)
        tan_values.append(tan_val)

    # Write each to their own CSV file
    with open('/mnt/data/sin_values_radians.csv', 'w', newline='') as sin_file:
        writer = csv.writer(sin_file)
        writer.writerow(["Radians", "Sin Value"])
        writer.writerows(zip(radians_list, sin_values))

    with open('/mnt/data/cos_values_radians.csv', 'w', newline='') as cos_file:
        writer = csv.writer(cos_file)
        writer.writerow(["Radians", "Cos Value"])
        writer.writerows(zip(radians_list, cos_values))

    with open('/mnt/data/tan_values_radians.csv', 'w', newline='') as tan_file:
        writer = csv.writer(tan_file)
        writer.writerow(["Radians", "Tan Value"])
        writer.writerows(zip(radians_list, tan_values))

# Generate the radian-based CSV files
generate_trig_csv_radians()
