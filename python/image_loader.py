import os

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
image_path = os.path.join(BASE_DIR, "data", "input", "test.png")

image = Image.open(image_path).convert("L")

print("Image loaded successfully!")
print("Width:", image.width)
print("Height:", image.height)
print("Mode:", image.mode)

pixels = list(image.getdata())

print("Total pixels:", len(pixels))
print("First 5 pixels:", pixels[:5])

output_path = os.path.join(BASE_DIR, "data", "input", "image.raw")

with open(output_path, "wb") as file:
    file.write(bytes(pixels))

print("Raw pixel file created!")