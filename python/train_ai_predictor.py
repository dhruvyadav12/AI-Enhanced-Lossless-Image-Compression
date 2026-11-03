import numpy as np
import os


# ==================================================
# CONFIGURATION
# ==================================================

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RAW_PATH = os.path.join(BASE_DIR, "data", "input", "image.raw")

WIDTH = 1086
HEIGHT = 1448


# ==================================================
# LOAD IMAGE
# ==================================================

with open(RAW_PATH, "rb") as file:
    data = file.read()


pixels = np.frombuffer(
    data,
    dtype=np.uint8
)


print("Image loaded successfully!")
print("Total pixels:", len(pixels))


# ==================================================
# CHECK IMAGE SIZE
# ==================================================

expected_pixels = WIDTH * HEIGHT

if len(pixels) != expected_pixels:

    print("ERROR!")
    print("Expected:", expected_pixels)
    print("Got:", len(pixels))

    exit()


# Convert 1D → 2D image

image = pixels.reshape(
    HEIGHT,
    WIDTH
)


print("Image shape:", image.shape)


# ==================================================
# CREATE TRAINING SAMPLES
# ==================================================

X = []
Y = []


# We skip the first row and first column
# because they don't have all three neighbors.

for row in range(1, HEIGHT):

    for col in range(1, WIDTH):

        left = int(
            image[row, col - 1]
        )

        top = int(
            image[row - 1, col]
        )

        top_left = int(
            image[row - 1, col - 1]
        )

        current_pixel = int(
            image[row, col]
        )


        # Input to neural network
        X.append([
            left,
            top,
            top_left
        ])


        # Target
        Y.append(
            current_pixel
        )


# ==================================================
# CONVERT TO NUMPY
# ==================================================

X = np.array(
    X,
    dtype=np.float32
)

Y = np.array(
    Y,
    dtype=np.float32
)


# ==================================================
# DISPLAY DATASET INFORMATION
# ==================================================

print()
print("========== DATASET ==========")

print("Input shape:", X.shape)

print("Target shape:", Y.shape)

print()


# ==================================================
# DISPLAY FIRST 5 SAMPLES
# ==================================================

print("First 5 samples:")

for i in range(5):

    print(
        "Input:",
        X[i],
        "Target:",
        Y[i]
    )


print("=============================")