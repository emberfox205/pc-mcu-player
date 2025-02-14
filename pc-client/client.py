from PIL import Image 
import serial

FRAME_FIRST = 43
FRAME_LAST = 6518

# Connect to simulation using RFC2217 through port 4000
# Baud rate must be consistent with C++ embedded codes
ser = serial.serial_for_url('rfc2217://localhost:4000', baudrate=115200, timeout=5)

# Import frames
img_array = list()
for index in range(FRAME_FIRST, FRAME_LAST + 1): 
    # This is just my own naming scheme
    img = Image.open(f"badapple/xbm_frames/frame_{index:04}.xbm")
    img_array.append(img)

img_bytes_array = list()
for img in img_array:
    # List of pixel values, normalized to binary values
    pixels = [x if x <= 128 else 1 for x in list(img.getdata())]

    # Convert to byte array
    pixel_bytes = list()
    # zip() creates tuples of 8 binary integers
    for byte in zip(*(iter(pixels),) * 8):
        pixel_bytes.append(sum([byte[b] << b for b in range(0, 8)]))
    pixel_bytes = bytes(pixel_bytes)
    img_bytes_array.append(pixel_bytes)

# Wait for signal to confirm setup is done 
while True: 
    frame_signal = ser.read_until('S',1).decode("utf-8").rstrip()
    if (frame_signal == 'S'):
        print("MCU ready")
        break

# Loop through each byte array
for bytes_frame in img_bytes_array:
    ser.write(bytes_frame)
    # Wait for confirming signal before continue sending
    while True:
        chunk_signal = ser.read_until('F',1).decode("utf-8").rstrip()
        if (chunk_signal == 'F'):
            print("Frame arrived")
            break
    
# Disconnect
ser.close()
