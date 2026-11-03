import os
import sys
import uvicorn

# Add current directory to path
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.insert(0, current_dir)

from python.api.main import app

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8000))
    host = os.environ.get("HOST", "0.0.0.0")
    print(f"Starting AI Lossless Image Compressor on http://{host}:{port}")
    uvicorn.run(app, host=host, port=port)
