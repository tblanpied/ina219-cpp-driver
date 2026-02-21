#!/bin/bash

UF2_FILE="build/ina219_pico.uf2"
PICO_MOUNT="/media/$USER/RPI-RP2" # For drag-and-drop fallback

function show_help() {
    echo "Usage: ./flash.sh [OPTIONS]"
    echo
    echo "Options:"
    echo "  --file <path>     Specify custom .uf2 file to flash"
    echo "  -h, --help        Show this help message and exit"
    echo
    echo "Examples:"
    echo "  ./flash.sh"
    echo "  ./flash.sh --file build/custom_firmware.uf2"
}

function check_uf2_exists() {
    if [ ! -f "$UF2_FILE" ]; then
        echo "❌ UF2 file not found at: $UF2_FILE"
        echo "👉 Use './build.sh' to build the firmware first."
        exit 1
    fi
}

function flash_with_picotool() {
    echo "⚡ Flashing firmware using picotool..."
    picotool load "$UF2_FILE" -f
    if [ $? -eq 0 ]; then
        echo "✅ Flash successful!"
    else
        echo "❌ Flash failed. Trying fallback method..."
        fallback_flash
    fi
}

function fallback_flash() {
    if [ -d "$PICO_MOUNT" ]; then
        echo "💾 Copying UF2 to Pico's mass storage..."
        cp "$UF2_FILE" "$PICO_MOUNT/"
        echo "✅ Flash complete via drag-and-drop!"
    else
        echo "❌ Pico not found as USB drive. Please press and hold BOOTSEL while plugging it in."
        exit 1
    fi
}

# Parse options
while [[ $# -gt 0 ]]; do
    case $1 in
        --file)
            shift
            UF2_FILE="$1"
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "❌ Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
    shift
done

check_uf2_exists
flash_with_picotool
