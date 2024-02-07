"""DDLC Mod Installer"""
# pylint: disable=no-member
# pyright: reportGeneralTypeIssues=false
import tkinter as tk
from tkinter import ttk
from tkinter import filedialog, messagebox
import os
import zipfile
import shutil
import winreg
import subprocess
import eel

eel.init('web')

LEFT_PADDING = 10
def calculate_directory_size(path):
    """Calculate the total size of all files in the directory."""
    total_size = 0
    for root, dirs, files in os.walk(path):
        total_size += sum(os.path.getsize(os.path.join(root, name)) for name in files)
    return total_size

def delete_directory_with_progress(path, total_size, deleted_size=None):
    """Delete a directory and its contents, updating the progress bar."""
    if deleted_size is None:
        deleted_size=[0]
    if not os.path.exists(path):
        return

    for root, dirs, files in os.walk(path, topdown=False):
        for name in files:
            file_path = os.path.join(root, name)
            file_size = os.path.getsize(file_path)
            os.remove(file_path)
            deleted_size[0] += file_size
            eel.updateProgressBar((deleted_size[0] / total_size) * 100)

        for name in dirs:
            os.rmdir(os.path.join(root, name))

    os.rmdir(path)  # Finally, remove the root directory itself
    eel.updateProgressBar(100)  # Ensure it reaches 100% at the end

def delete_ddlc(game_path):
    "Delete the DDLC Folder for a Clean Install"

    # Check if the path is empty
    if not game_path:
        eel.showMessage("Error", "Game directory is empty. Please specify a valid path.")
        return

    # Safeguard checks to ensure the directory is indeed for DDLC
    valid_names = ["Doki Doki Literature Club"]
    if not any(name in game_path for name in valid_names):
        eel.showMessage(
            "Error", 
            "The specified directory does not appear to be a valid DDLC installation."
            )
        eel.appendToConsole("Error: Attempted to delete a non-DDLC directory.")
        return

    # Check for existence of expected game files as an extra precaution
    expected_files = ["DDLC.exe", "DDLC.sh", "game"]
    if not any(
        os.path.exists(
            os.path.join(game_path, expected_file)
        ) for expected_file in expected_files):
        eel.showMessage(
            "Error",
            "The specified directory does not contain expected DDLC files.")
        eel.appendToConsole("Error: The specified directory lacks expected DDLC files.")
        return

    # Confirmation dialog
    confirm = messagebox.askyesno(
        "Confirm Uninstall",
        "Are you sure you want to Uninstall DDLC? This action cannot be undone!", icon='warning')
    if confirm:
        try:
            total_size = calculate_directory_size(game_path)
            eel.showProgressBar()  # Initialize progress bar
            delete_directory_with_progress(game_path, total_size)
            eel.appendToConsole(f"DDLC has been uninstalled successfully from: {game_path}")
            messagebox.showinfo("Uninstall Complete", "DDLC has been successfully uninstalled.")
            game_path_entry.delete(0, tk.END)  # Clear the path entry after deletion
        except Exception as e:
            eel.appendToConsole(f"Error during uninstallation: {e}")
            eel.showMessage("Error", f"Failed to uninstall DDLC. {e}")
    else:
        eel.appendToConsole("Uninstallation cancelled.")


def get_steam_path():
    """Find Steam on the System
    returns: Steam Path"""
    try:
        key_path = r"SOFTWARE\Valve\Steam"
        if os.environ["PROCESSOR_ARCHITECTURE"].endswith('64'):
            key_path = r"SOFTWARE\Wow6432Node\Valve\Steam"
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key_path, 0, winreg.KEY_READ)
        value, _ = winreg.QueryValueEx(key, "InstallPath")
        eel.appendToConsole(f"Steam Path Value: {value}")
        return value
    except Exception as e:
        eel.appendToConsole(f"Error accessing registry: {e}")
        return None

def parse_vdf_for_paths(vdf_path, game_ids=None):
    """Find the Game Path in the steam library"""
    if game_ids is None:
        game_ids=['698780']
    paths = []
    try:
        with open(vdf_path, 'r', encoding='utf-8') as file:
            lines = file.readlines()

        current_path = ""
        in_apps_block = False
        for line in lines:
            # Strip line of whitespace for cleaner processing
            stripped_line = line.strip()
            if '"path"' in stripped_line:
                # Extract path value
                current_path = stripped_line.split('"')[3].replace('\\\\', '\\')
            if '"apps"' in stripped_line:
                in_apps_block = True
            elif in_apps_block:
                if any(game_id in stripped_line for game_id in game_ids):
                    # Found a game ID, add the current path if it's not already added
                    if current_path and current_path not in paths and os.path.exists(current_path):
                        paths.append(current_path)
                        eel.appendToConsole(f"Found Steam library with game: {current_path}")
                        # Reset for next library
                        current_path = ""
                        in_apps_block = False
                elif '}' in stripped_line:
                    # Exiting the apps block
                    in_apps_block = False
                    current_path = ""  # Reset current path after exiting an apps block

    except Exception as e:
        eel.appendToConsole(f"Error parsing VDF: {e}")
    eel.appendToConsole(f"VDF Paths: {paths}")
    return paths


def calculate_total_size(zip_path):
    """Get total size of zip"""
    total_size = 0
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        for file_info in zip_ref.infolist():
            total_size += file_info.file_size
    return total_size

def find_game_directory():
    """Find the Directory of DDLC"""
    steam_path = get_steam_path()
    if steam_path:
        vdf_path = os.path.join(steam_path, "steamapps", "libraryfolders.vdf")
        library_paths = parse_vdf_for_paths(vdf_path)
        library_paths.insert(0, steam_path)  # Include the default path
        for path in library_paths:
            for game_folder in ["Doki Doki Literature Club"]:
                game_path = os.path.join(path, "steamapps", "common", game_folder)
                eel.appendToConsole(f"Game Path: {game_path}")
                if os.path.exists(game_path):
                    return game_path
    # Return an informative string or empty if not found
    return "Game directory not found automatically."

def merge_directories(src, dst, processed_size, destination_path):
    """Merge directories from src to dst, overwriting conflicts, and update progress bar."""
    dst = os.path.join(destination_path, dst)  # Adjust destination based on user choice
    for root, dirs, files in os.walk(src):
        # Calculate relative path to the source directory
        rel_path = os.path.relpath(root, src)
        dst_path = os.path.join(dst, rel_path)

        # Ensure the destination directory exists
        os.makedirs(dst_path, exist_ok=True)

        # Copy and overwrite files
        for file in files:
            src_file_path = os.path.join(root, file)
            dst_file_path = os.path.join(dst_path, file)
            file_size = os.path.getsize(src_file_path)
            if os.path.exists(dst_file_path):
                eel.appendToConsole(f"Overwriting file: {dst_file_path}")
            else:
                eel.appendToConsole(f"Copying file: {dst_file_path}")
            shutil.copy2(src_file_path, dst_file_path)
            processed_size[0] += file_size  # Update processed size
            eel.updateProgressBar(processed_size[0])



def overwrite_file(src, dst, processed_size, destination_path, total_size):
    """Overwrite the file at dst with src, within the destination path."""
    dst = os.path.join(destination_path, dst)  # Adjust destination path
    file_size = os.path.getsize(src)
    if os.path.exists(dst):
        os.remove(dst)
        eel.appendToConsole(f"Removed existing file: {dst}")
    shutil.copy2(src, dst)
    eel.appendToConsole(f"Copied {src} to {dst}")
    processed_size[0] += file_size
    eel.updateProgressBar((processed_size[0] / total_size) * 100)

@eel.expose
def process_files(zip_path, game_path, separate_mod_path=None):
    """Process files and directories with explicit handling for overwriting."""
    open_dir = False

    print(zip_path)
    print(game_path)
    print(separate_mod_path)
    eel.appendToConsole(f"Processing files from: {zip_path} to {game_path}")

    if not zip_path.lower().endswith('.zip'):
        eel.appendToConsole("Error: The provided path does not point to a zip file.")
        return

    destination_path = separate_mod_path if separate_mod_path else game_path

    try:


        game_dir_size = calculate_directory_size(game_path)
        mod_file_size = calculate_total_size(zip_path)
        total_size = game_dir_size + mod_file_size

        # Initialize progress tracking
        processed_size = [0]

        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            extract_path = os.path.splitext(zip_path)[0]
            try:
                zip_ref.extractall(extract_path)
                eel.appendToConsole(f"Extracted zip to: {extract_path}")
            except PermissionError:
                eel.appendToConsole("Warning: Permission denied during extraction.")
                return
        if separate_mod_path is not None:
            copy_game_files(
                game_path,
                separate_mod_path,
                processed_size,
                total_size)
        open_dir = process_extracted_files(
            extract_path,
            destination_path,
            processed_size,
            total_size,
            destination_path
            )

    except Exception as e:
        eel.appendToConsole(f"Error during processing: {e}")
        eel.showMessage("Error", f"An error occurred: {e}")

    eel.updateProgressBar(100)  # Ensure progress bar reaches 100% at the end
    eel.enableUIElements()
    messagebox.showinfo("Process Completed", "All files have been processed successfully.")
    if open_dir:
        destination_path_abs = os.path.abspath(destination_path)
        if os.path.exists(destination_path):  # Make sure the path exists before trying to open it
            subprocess.run(['explorer', destination_path_abs], check=True)
        else:
            eel.showMessage("Error", "The specified path does not exist.")


def copy_game_files(game_path, destination_path, processed_size, total_size):
    """Copy all game files to the destination directory and update progress."""
    for item in os.listdir(game_path):
        src_path = os.path.join(game_path, item)
        dst_path = os.path.join(destination_path, item)
        if os.path.isdir(src_path):
            for root, dirs, files in os.walk(src_path):
                for file in files:
                    file_src_path = os.path.join(root, file)
                    file_dst_path = os.path.join(dst_path, os.path.relpath(root, src_path), file)
                    os.makedirs(os.path.dirname(file_dst_path), exist_ok=True)
                    shutil.copy2(file_src_path, file_dst_path)
                    file_size = os.path.getsize(file_src_path)
                    processed_size[0] += file_size
                    eel.updateProgressBar((processed_size[0] / total_size) * 100)
        else:
            if not os.path.exists(os.path.dirname(dst_path)):
                os.makedirs(os.path.dirname(dst_path), exist_ok=True)
            shutil.copy2(src_path, dst_path)
            file_size = os.path.getsize(src_path)
            processed_size[0] += file_size
            eel.updateProgressBar((processed_size[0] / total_size) * 100)
    eel.appendToConsole(f"Copied game files to: {destination_path}")



def process_extracted_files(extract_path, game_path, processed_size, total_size, destination_path=None):
    """Process Game files after zip extraction."""
    open_dir = False
    target_files = ['audio.rpa', 'fonts.rpa', 'images.rpa', 'scripts.rpa']
    target_dirs = ['game', 'characters', 'lib', 'renpy']
    executable_extensions = ['.exe', '.bat', '.sh', '.py']

    if destination_path is None:
        destination_path = game_path  # Use game path if no separate mod path is provided

    # Find the directory that contains any of the target directories or files
    for root, dirs, files in os.walk(extract_path):
        # Check if any of the target directories or target files are present in the current 'root'
        if any(
            t_dir in dirs for t_dir in target_dirs
              ) or any(
                t_file in files for t_file in target_files
                ):
            base_dir = root
            break
    else:
        eel.appendToConsole("None of the target directories or files found in the extracted path.")
        return open_dir

    # Process files and directories from the found base directory
    for root, dirs, files in os.walk(base_dir, topdown=True):
        # Files processing
        for name in files:
            src_path = os.path.join(root, name)
            # Determine if the file is an executable or script and handle accordingly
            if any(name.lower().endswith(ext) for ext in executable_extensions):
                # Executables go directly to destination_path
                dst_path = name
                eel.appendToConsole(f"Moving executable/script: {name}")
                overwrite_file(src_path, dst_path, processed_size, destination_path, total_size)
                if name.lower().endswith('.exe'):
                    open_dir = True
            elif name in target_files:
                # Target files go to 'game' directory inside destination_path
                dst_path = os.path.join('game', name)
                eel.appendToConsole(f"Moving target file: {name} to {dst_path}")
                overwrite_file(src_path, dst_path, processed_size, destination_path, total_size)

        # Directories processing
        for name in dirs:
            src_path = os.path.join(root, name)
            if name in target_dirs or name.endswith('.app'):
                # Target directories are copied to the destination_path
                eel.appendToConsole(f"Copying directory: {name}")
                merge_directories(src_path, name, processed_size, destination_path)

        # Modify dirs list to exclude the target directories since they are already processed
        dirs[:] = [d for d in dirs if d not in target_dirs and not d.endswith('.app')]

    return open_dir

# ======================
#      UI Functions
# ======================

@eel.expose
def browse_path(is_folder):
    """Open a dialog to update the entry with the selected path,
    either file or folder, and log to console."""
    root = tk.Tk()
    root.withdraw()  # Hide the Tkinter root window
    if is_folder:
        filename = filedialog.askdirectory()
        action = "Directory selected: "
    else:
        filename = filedialog.askopenfilename()
        action = "File selected: "
    return {'path': filename, 'action': action} if filename else None

@eel.expose
def auto_toggle():
    """Toggle the entry field for automatic game location detection and log to console."""
    game_path = find_game_directory()
    message = "Auto detection enabled with path: " + game_path
    return {'message': message, 'gamePath': game_path}



# =====================
#     UI Widgets
# =====================


# App Setup

eel.start('index.html', size=(800, 700))