from PIL import Image
import os

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RAW_PATH = os.path.join(BASE_DIR, "data", "output", "reconstructed.raw")
OUTPUT_PATH = os.path.join(BASE_DIR, "data", "output", "reconstructed.png")

WIDTH = 1086
HEIGHT = 1448


with open(RAW_PATH, "rb") as file:
    data = file.read()


expected_size = WIDTH * HEIGHT


print("Raw bytes:", len(data))
print("Expected bytes:", expected_size)


if len(data) != expected_size:

    print("ERROR: Size mismatch!")

    exit()


image = Image.frombytes(
    "L",
    (WIDTH, HEIGHT),
    data
)


image.save(
    OUTPUT_PATH
)


print()
print("Reconstructed PNG created successfully!")
print("Output:")
print(OUTPUT_PATH)