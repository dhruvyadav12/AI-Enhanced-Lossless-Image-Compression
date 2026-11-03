import numpy as np
import torch
import torch.nn as nn


# ==================================================
# CONFIGURATION
# ==================================================

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

print("AI model loaded successfully!")


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


print("Image loaded successfully!")
print("Image shape:", image.shape)


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

        targets.append(
            current
        )


X = np.array(
    X,
    dtype=np.float32
)

targets = np.array(
    targets,
    dtype=np.float32
)


# ==================================================
# NORMALIZE INPUT
# ==================================================

X_normalized = X / 255.0


X_tensor = torch.tensor(
    X_normalized
)


# ==================================================
# AI PREDICTION
# ==================================================

with torch.no_grad():

    predictions = model(
        X_tensor
    )


predictions = (
    predictions
    .numpy()
    .flatten()
)


# Convert back to pixel range

predicted_pixels = np.rint(
    predictions * 255
)


# Clamp to valid pixel range

predicted_pixels = np.clip(
    predicted_pixels,
    0,
    255
)


# ==================================================
# RESIDUALS
# ==================================================

residuals = (
    targets -
    predicted_pixels
)


absolute_residuals = np.abs(
    residuals
)


# ==================================================
# METRICS
# ==================================================

average_abs_residual = np.mean(
    absolute_residuals
)


zero_residuals = np.sum(
    residuals == 0
)


zero_percentage = (
    100.0 *
    zero_residuals /
    len(residuals)
)


maximum_residual = np.max(
    absolute_residuals
)


# ==================================================
# RESULTS
# ==================================================

print()
print("========== AI PREDICTOR ==========")

print(
    "Average absolute residual:",
    average_abs_residual
)

print(
    "Zero residuals:",
    zero_residuals
)

print(
    "Zero residual percentage:",
    zero_percentage,
    "%"
)

print(
    "Maximum absolute residual:",
    maximum_residual
)


# ==================================================
# FIRST 10 PREDICTIONS
# ==================================================

print()
print("First 10 AI predictions:")

for i in range(10):

    print(
        "Predicted:",
        int(predicted_pixels[i]),
        "Actual:",
        int(targets[i]),
        "Residual:",
        int(residuals[i])
    )


print("==================================")