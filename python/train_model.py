import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import TensorDataset, DataLoader


# ==================================================
# CONFIGURATION
# ==================================================

import os

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RAW_PATH = os.path.join(BASE_DIR, "data", "input", "image.raw")
MODEL_PATH = os.path.join(BASE_DIR, "data", "output", "predictor_model.pth")

WIDTH = 1086
HEIGHT = 1448

EPOCHS = 10
BATCH_SIZE = 4096
LEARNING_RATE = 0.001


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


print("Image loaded.")
print("Image shape:", image.shape)


# ==================================================
# CREATE TRAINING DATA
# ==================================================

X = []
Y = []


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

        Y.append(current)


X = np.array(
    X,
    dtype=np.float32
)

Y = np.array(
    Y,
    dtype=np.float32
)


# ==================================================
# NORMALIZE
# ==================================================

X = X / 255.0
Y = Y / 255.0


# ==================================================
# CONVERT TO PYTORCH
# ==================================================

X_tensor = torch.tensor(X)

Y_tensor = torch.tensor(Y).reshape(-1, 1)


dataset = TensorDataset(
    X_tensor,
    Y_tensor
)


loader = DataLoader(
    dataset,
    batch_size=BATCH_SIZE,
    shuffle=True
)


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


model = Predictor()


# ==================================================
# LOSS + OPTIMIZER
# ==================================================

criterion = nn.MSELoss()

optimizer = torch.optim.Adam(
    model.parameters(),
    lr=LEARNING_RATE
)


# ==================================================
# TRAIN
# ==================================================

print()
print("========== TRAINING ==========")


for epoch in range(EPOCHS):

    total_loss = 0.0


    for inputs, targets in loader:

        predictions = model(inputs)

        loss = criterion(
            predictions,
            targets
        )


        optimizer.zero_grad()

        loss.backward()

        optimizer.step()


        total_loss += loss.item()


    average_loss = (
        total_loss / len(loader)
    )


    print(
        f"Epoch {epoch + 1}/{EPOCHS} "
        f"Loss: {average_loss:.6f}"
    )


# ==================================================
# SAVE MODEL
# ==================================================

torch.save(
    model.state_dict(),
    MODEL_PATH
)


print()
print("Model trained successfully!")

print("Model saved to:")

print(MODEL_PATH)