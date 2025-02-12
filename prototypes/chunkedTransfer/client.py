from PIL import Image 
import serial, time

CHUNK_SIZE = 256
FRAME_FIRST = 5431
FRAME_LAST = 5478

# Connect to simulation using RFC2217 through port 4000
ser = serial.serial_for_url('rfc2217://localhost:4000', baudrate=921600)

# Import frames
img_array = list()
for index in range(FRAME_FIRST, FRAME_LAST + 1): 
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
    frame_signal = ser.read_until('F', 1).decode("utf-8").rstrip()
    if (frame_signal == 'F'):
        print("Frame buffer ready")
        break

start = time.time()
for bytes_frame in img_bytes_array:
    # Send bytes in chunks.
    for i in range(0, len(bytes_frame), CHUNK_SIZE):
        chunk = bytes_frame[i:i+CHUNK_SIZE]
        # Add an end marker for final chunk of frame
        if (len(chunk) < CHUNK_SIZE):
            chunk += b'\n'
        ser.write(chunk)
        # Wait for signal to confirm chunk is received and stored
        while True:
            chunk_signal = ser.read_until('C',1).decode("utf-8").rstrip()
            if (chunk_signal == 'C'):
                print("Chunk arrived")
                break

end = time.time()
# Disconnect
ser.close()
print(f"FPS = {(FRAME_LAST - FRAME_FIRST)/(end-start)}")
