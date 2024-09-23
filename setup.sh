#!/bin/bash
echo "Starting setup..."

# Update package list
sudo apt update

# Install REQUIRED dependencies
sudo apt install -y build-essential libgl1-mesa-dev
sudo apt install -y libglfw3 libglfw3-dev
sudo apt install -y libglew-dev
sudo apt install -y libglm-dev
sudo apt install -y libpcl-dev

# Clean up any residual files
sudo apt autoremove -y

# Done
echo "Installation of dependencies complete!"
