import numpy as np
import torch
import torch.nn as nn
from collections import Counter
import heapq


import os

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RAW_PATH = os.path.join(BASE_DIR, "data", "input", "image.raw")
MODEL_PATH = os.path.join(BASE_DIR, "data", "output", "predictor_model.pth")

WIDTH = 1086
HEIGHT = 1448


# ==================================================
# MODEL
# ==================================================

class Predictor(nn.Module):

    def __init__(self):

        super().__init__()

        self.network = nn.Sequential(
            nn.Linear(3, 32),
            nn.ReLU(),
            nn.Linear(32, 32),
            nn.ReLU(),
            nn.Linear(32, 1)
        )

    def forward(self, x):
        return self.network(x)


# ==================================================
# LOAD MODEL
# ==================================================

model = Predictor()

model.load_state_dict(
    torch.load(
        MODEL_PATH,
        map_location="cpu"
    )
)

model.eval()


# ==================================================
# LOAD IMAGE
# ==================================================

with open(RAW_PATH, "rb") as file:
    data = file.read()


pixels = np.frombuffer(
    data,
    dtype=np.uint8
)

image = pixels.reshape(
    HEIGHT,
    WIDTH
)


# ==================================================
# CREATE INPUTS
# ==================================================

X = []

targets = []


for row in range(1, HEIGHT):

    for col in range(1, WIDTH):

        left = image[row, col - 1]

        top = image[row - 1, col]

        top_left = image[row - 1, col - 1]

        current = image[row, col]

        X.append([
            left,
            top,
            top_left
        ])

        targets.append(current)


X = np.array(
    X,
    dtype=np.float32
)

targets = np.array(
    targets,
    dtype=np.float32
)


# ==================================================
# AI PREDICTION
# ==================================================

X_tensor = torch.tensor(
    X / 255.0
)


with torch.no_grad():

    predictions = model(
        X_tensor
    ).numpy().flatten()


predicted = np.rint(
    predictions * 255
)

predicted = np.clip(
    predicted,
    0,
    255
)


# ==================================================
# IMPORTANT:
# FIRST ROW + FIRST COLUMN
# ==================================================

# Our AI predictor does not process the first row
# and first column.

# For a fair full-image benchmark, use the same
# simple prediction as the classical compressor
# for those pixels.

full_predictions = image.copy().astype(np.int16)


# First pixel
full_predictions[0, 0] = 0


# First row
for col in range(1, WIDTH):

    full_predictions[0, col] = \
        image[0, col - 1]


# First column
for row in range(1, HEIGHT):

    full_predictions[row, 0] = \
        image[row - 1, 0]


# AI predictions for remaining pixels

index = 0

for row in range(1, HEIGHT):

    for col in range(1, WIDTH):

        full_predictions[row, col] = \
            predicted[index]

        index += 1


# ==================================================
# RESIDUALS
# ==================================================

residuals = (
    image.astype(np.int16)
    -
    full_predictions
)


# ==================================================
# ZIGZAG
# ==================================================

def zigzag(value):

    if value >= 0:
        return 2 * value

    return -2 * value - 1


zigzag_values = np.array(
    [zigzag(int(x)) for x in residuals.flatten()],
    dtype=np.int32
)


# ==================================================
# FREQUENCY TABLE
# ==================================================

frequency = Counter(
    zigzag_values.tolist()
)


print()
print("========== AI COMPRESSION ==========")

print(
    "Unique symbols:",
    len(frequency)
)


# ==================================================
# HUFFMAN TREE
# ==================================================

heap = []

counter = 0


for value, freq in frequency.items():

    heapq.heappush(
        heap,
        (freq, counter, value)
    )

    counter += 1


# Special case: only one symbol

if len(heap) == 1:

    total_bits = len(zigzag_values)

else:

    # Each heap item:
    # (frequency, unique_id, node)

    nodes = {}

    while len(heap) > 1:

        freq1, id1, node1 = heapq.heappop(heap)

        freq2, id2, node2 = heapq.heappop(heap)

        parent = (
            node1,
            node2
        )

        heapq.heappush(
            heap,
            (
                freq1 + freq2,
                counter,
                parent
            )
        )

        counter += 1


    root = heap[0][2]


    # Calculate code lengths

    code_lengths = {}


    def traverse(node, depth):

        if not isinstance(node, tuple):

            code_lengths[node] = depth

            return

        traverse(
            node[0],
            depth + 1
        )

        traverse(
            node[1],
            depth + 1
        )


    traverse(root, 0)


    # Calculate total bits

    total_bits = 0

    for value, freq in frequency.items():

        total_bits += (
            freq *
            code_lengths[value]
        )


# ==================================================
# RESULTS
# ==================================================

compressed_bytes = (
    total_bits + 7
) // 8


original_bytes = len(pixels)


ratio = (
    original_bytes /
    compressed_bytes
)


reduction = (
    100 *
    (
        1 -
        compressed_bytes /
        original_bytes
    )
)


print(
    "Original size:",
    original_bytes,
    "bytes"
)

print(
    "Compressed payload:",
    compressed_bytes,
    "bytes"
)

print(
    "Compressed bits:",
    total_bits
)

print(
    "Compression ratio:",
    ratio,
    "x"
)

print(
    "Size reduction:",
    reduction,
    "%"
)

print(
    "===================================="
)