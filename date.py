# -*- coding: utf-8 -*-
"""
Created on Fri May  9 20:30:20 2025

@author: Alex Razvi
"""

import os
import pandas as pd

# Define the three known folders to search inside each subfolder
KNOWN_FOLDERS = ["resolution_naive_first_fit.exe_results","clauseDrivenConflictLearning.exe_results", "dpll_mcl.exe_results", "dp_naive.exe_results"]

def get_second_parent_folder(file_path):
    """ Get the second parent folder name """
    abs_path = os.path.abspath(file_path)
    parents = abs_path.split(os.sep)
    
    if len(parents) > 2:
        return parents[-3]  # Second parent folder
    else:
        return "No second parent folder"

def process_csv(file_path):
    """ Read CSV and process data """
    try:
        df = pd.read_csv(file_path)

        # Remove last two characters from 'time' column and convert to float
        df["Time"] = df["Time"].apply(lambda x: int(''.join(filter(str.isdigit, x))))
        #df["Time"] = df["Time"].astype(str).str[:-4].astype(int)
        print(df["Time"][:-4])
        # Compute statistics for 'time' column
        avg_time = df["Time"].mean()
        min_time = df["Time"].min()
        max_time = df["Time"].max()

        # Compute statistics for 'Memory' column
        df["Memory(B)"] = df["Memory(B)"].astype(str).str[:-1].astype(int)
        avg_memory = df["Memory(B)"].mean()
        min_memory = df["Memory(B)"].min()
        max_memory = df["Memory(B)"].max()
        
        return avg_time, min_time, max_time, avg_memory, min_memory, max_memory
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return None

def scan_main_folder(main_folder):
    """ Scan through the main folder and process CSV files """
    
    # Create a dictionary to store data for each known folder
    folder_data = {folder: [] for folder in KNOWN_FOLDERS}
    
    # Iterate through each subfolder
    for subfolder in os.listdir(main_folder):
        subfolder_path = os.path.join(main_folder, subfolder)
        
        if os.path.isdir(subfolder_path):  # Ensure it's a folder
            for known_folder in KNOWN_FOLDERS:
                known_folder_path = os.path.join(subfolder_path, known_folder)
                
                csv_file = os.path.join(known_folder_path, "benchmark_results.csv")
                
                if os.path.exists(csv_file):
                    results = process_csv(csv_file)
                    if results is not None:
                        avg_time, min_time, max_time, avg_memory, min_memory, max_memory = results
                        folder_name = get_second_parent_folder(csv_file)
                        folder_data[known_folder].append({
                            "Folder Name": folder_name,
                            "Timp Mediu": avg_time,
                            "Min Timp": min_time,
                            "Max Timp": max_time,
                            "Memorie Medie": avg_memory,
                            "Min Memorie": min_memory,
                            "Max Memorie": max_memory
                        })
    # Save separate CSV files for each known folder
    for folder, data in folder_data.items():
        output_df = pd.DataFrame(data)
        print(data)
        output_csv = f"{folder}_output.csv"
        output_df.to_csv(output_csv, index=False)
        print(f"Results for {folder} saved to {output_csv}")

# Example usage
main_folder_path = R"D:\Project-MPI\TESTE"  # Change this to your actual folder path
scan_main_folder(main_folder_path)