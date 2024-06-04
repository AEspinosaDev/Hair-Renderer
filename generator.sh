#!/bin/bash 


echo "Marschner LUT Generator"

is_number() {
    re='^[0-9]+$'
    [[ $1 =~ $re ]]
}


# Prompt for texture size
while true; do
    read -p "Enter LUT size (e.g., 256, 512, 1024): " texture_size
    if is_number "$texture_size"; then
        break
    else
        echo "Invalid input. Please enter a valid number."
    fi
done

# Prompt for beta term
while true; do
    read -p "Enter beta term: " beta
    if is_number "$beta"; then
        break
    else
        echo "Invalid input. Please enter a valid number."
    fi
done

# Prompt for shift term
while true; do
    read -p "Enter shift term: " shift
    if is_number "$shift"; then
        break
    else
        echo "Invalid input. Please enter a valid number."
    fi
done


start_time=$(date +%s)
./build/bin/debug/LUTGenerator --size "$texture_size" --beta "$beta" --shift "$shift" 
end_time=$(date +%s)

execution_time=$((end_time - start_time))

echo "Execution completed in $execution_time seconds."
