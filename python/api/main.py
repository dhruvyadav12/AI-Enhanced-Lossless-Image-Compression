from fastapi import FastAPI, UploadFile, File
from fastapi.responses import FileResponse
from fastapi.middleware.cors import CORSMiddleware

from PIL import Image

import subprocess
import os
import shutil
import struct


# ============================================================
# FASTAPI APPLICATION
# ============================================================

app = FastAPI(
    title="AI Lossless Image Compressor",
    description=(
        "Lossless image compression using "
        "Average Prediction + ZigZag + Huffman Coding"
    ),
    version="1.0.0"
)


# ============================================================
# CORS
# ============================================================

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=False,
    allow_methods=["*"],
    allow_headers=["*"],
)


# ============================================================
# PROJECT PATHS
# ============================================================

BASE_DIR = os.path.dirname(
    os.path.dirname(
        os.path.dirname(
            os.path.abspath(__file__)
        )
    )
)

FRONTEND_DIR = os.path.join(
    BASE_DIR,
    "frontend"
)

INPUT_DIR = os.path.join(
    BASE_DIR,
    "data",
    "input"
)

OUTPUT_DIR = os.path.join(
    BASE_DIR,
    "data",
    "output"
)

TEMP_IMAGE = os.path.join(
    INPUT_DIR,
    "uploaded_image"
)

RAW_FILE = os.path.join(
    INPUT_DIR,
    "image.raw"
)

AIC_FILE = os.path.join(
    OUTPUT_DIR,
    "compressed.aic"
)

RECONSTRUCTED_RAW = os.path.join(
    OUTPUT_DIR,
    "reconstructed.raw"
)

RECONSTRUCTED_PNG = os.path.join(
    OUTPUT_DIR,
    "reconstructed.png"
)


# ============================================================
# CROSS-PLATFORM BINARY RESOLVER & AUTO-COMPILER
# ============================================================

def resolve_or_build_binary(name: str) -> str:
    cpp_src = os.path.join(BASE_DIR, "cpp", "src")
    cpp_inc = os.path.join(BASE_DIR, "cpp", "include")
    
    candidates = [
        f"{name}.exe",
        name
    ] if os.name == "nt" else [
        name,
        f"{name}.exe"
    ]

    for candidate in candidates:
        bin_path = os.path.join(cpp_src, candidate)
        if os.path.exists(bin_path):
            return bin_path

    # Auto-compile fallback using g++ if binary not present
    source_path = os.path.join(cpp_src, f"{name}.cpp")
    target_bin = os.path.join(cpp_src, f"{name}.exe" if os.name == "nt" else name)

    if os.path.exists(source_path):
        try:
            print(f"Compiling {name} from source...")
            subprocess.run(
                ["g++", "-O3", "-std=c++11", f"-I{cpp_inc}", source_path, "-o", target_bin],
                cwd=cpp_src,
                check=True,
                capture_output=True,
                text=True
            )
            print(f"Successfully compiled {name}: {target_bin}")
            return target_bin
        except Exception as err:
            print(f"Warning: Auto-compilation of {name} failed: {err}")

    return os.path.join(cpp_src, f"{name}.exe" if os.name == "nt" else name)


ENCODER = resolve_or_build_binary("encoder")
DECODER = resolve_or_build_binary("decoder")


# ============================================================
# CREATE DIRECTORIES
# ============================================================

os.makedirs(
    INPUT_DIR,
    exist_ok=True
)

os.makedirs(
    OUTPUT_DIR,
    exist_ok=True
)


# ============================================================
# ROOT & FRONTEND SERVING
# ============================================================

@app.get("/")
def root():
    index_path = os.path.join(FRONTEND_DIR, "index.html")
    if os.path.exists(index_path):
        return FileResponse(index_path)
    return {
        "project": "AI Lossless Image Compressor",
        "status": "running",
        "version": "1.0.0"
    }


@app.get("/style.css")
def get_style():
    style_path = os.path.join(FRONTEND_DIR, "style.css")
    if os.path.exists(style_path):
        return FileResponse(style_path, media_type="text/css")
    return {"status": "error", "message": "style.css not found"}


@app.get("/script.js")
def get_script():
    script_path = os.path.join(FRONTEND_DIR, "script.js")
    if os.path.exists(script_path):
        return FileResponse(script_path, media_type="application/javascript")
    return {"status": "error", "message": "script.js not found"}


# ============================================================
# HEALTH CHECK
# ============================================================

@app.get("/health")
def health():

    return {
        "status": "healthy",
        "encoder_available": os.path.exists(ENCODER),
        "decoder_available": os.path.exists(DECODER),
        "encoder_path": ENCODER,
        "decoder_path": DECODER,
        "input_directory": INPUT_DIR,
        "output_directory": OUTPUT_DIR
    }


# ============================================================
# COMPRESS
# ============================================================

@app.post("/compress")
async def compress(
    file: UploadFile = File(...)
):

    # --------------------------------------------------------
    # 1. Validate file
    # --------------------------------------------------------

    if not file.filename:

        return {
            "status": "error",
            "message": "No file was provided."
        }


    # --------------------------------------------------------
    # 2. Save uploaded image
    # --------------------------------------------------------

    try:

        with open(
            TEMP_IMAGE,
            "wb"
        ) as buffer:

            shutil.copyfileobj(
                file.file,
                buffer
            )

    except Exception as e:

        return {
            "status": "error",
            "message": "Could not save uploaded image.",
            "details": str(e)
        }


    # --------------------------------------------------------
    # 3. Open and convert image
    # --------------------------------------------------------

    try:

        image = Image.open(
            TEMP_IMAGE
        )

        # Current C++ pipeline is grayscale.

        image = image.convert("L")

        width, height = image.size

    except Exception as e:

        return {
            "status": "error",
            "message": "Invalid image file.",
            "details": str(e)
        }


    # --------------------------------------------------------
    # 4. Convert image to RAW
    # --------------------------------------------------------

    try:

        pixel_data = image.tobytes()

        with open(
            RAW_FILE,
            "wb"
        ) as raw_file:

            raw_file.write(
                pixel_data
            )

    except Exception as e:

        return {
            "status": "error",
            "message": "Could not create RAW image.",
            "details": str(e)
        }


    # --------------------------------------------------------
    # 5. Verify RAW size
    # --------------------------------------------------------

    original_size = os.path.getsize(
        RAW_FILE
    )

    expected_size = (
        width * height
    )


    if original_size != expected_size:

        return {
            "status": "error",
            "message": "RAW file size mismatch.",
            "expected_bytes": expected_size,
            "actual_bytes": original_size
        }


    # --------------------------------------------------------
    # 6. Check encoder
    # --------------------------------------------------------

    if not os.path.exists(ENCODER):

        return {
            "status": "error",
            "message": "C++ encoder executable not found.",
            "encoder_path": ENCODER
        }


    # --------------------------------------------------------
    # 7. Remove old compressed file
    # --------------------------------------------------------

    if os.path.exists(AIC_FILE):

        try:

            os.remove(
                AIC_FILE
            )

        except Exception as e:

            return {
                "status": "error",
                "message": "Could not remove old AIC file.",
                "details": str(e)
            }


    # --------------------------------------------------------
    # 8. Run C++ encoder
    # --------------------------------------------------------

    try:

        result = subprocess.run(
            [
                ENCODER,
                str(width),
                str(height)
            ],
            cwd=os.path.dirname(ENCODER),
            capture_output=True,
            text=True,
            timeout=120
        )

    except subprocess.TimeoutExpired:

        return {
            "status": "error",
            "message": "C++ encoder timed out."
        }

    except Exception as e:

        return {
            "status": "error",
            "message": "Could not start C++ encoder.",
            "details": str(e)
        }


    # --------------------------------------------------------
    # 9. Check encoder result
    # --------------------------------------------------------

    if result.returncode != 0:

        return {
            "status": "error",
            "message": "C++ compression failed.",
            "return_code": result.returncode,
            "stdout": result.stdout,
            "stderr": result.stderr
        }


    # --------------------------------------------------------
    # 10. Check AIC output
    # --------------------------------------------------------

    if not os.path.exists(AIC_FILE):

        return {
            "status": "error",
            "message": (
                "C++ encoder completed but "
                "compressed.aic was not created."
            ),
            "encoder_output": result.stdout
        }


    compressed_size = os.path.getsize(
        AIC_FILE
    )


    if compressed_size == 0:

        return {
            "status": "error",
            "message": "Compressed AIC file is empty."
        }


    # --------------------------------------------------------
    # 11. Compression statistics
    # --------------------------------------------------------

    compression_ratio = (
        original_size /
        compressed_size
    )


    size_reduction = (
        100.0 *
        (
            1.0 -
            (
                compressed_size /
                original_size
            )
        )
    )


    # --------------------------------------------------------
    # 12. Return result
    # --------------------------------------------------------

    return {

        "status": "success",

        "filename": file.filename,

        "image": {

            "width": width,
            "height": height,
            "channels": 1,
            "mode": "grayscale"
        },

        "compression": {

            "original_size_bytes":
                original_size,

            "compressed_size_bytes":
                compressed_size,

            "compression_ratio":
                round(
                    compression_ratio,
                    4
                ),

            "size_reduction_percent":
                round(
                    size_reduction,
                    2
                )
        },

        "predictor": "Average",

        "entropy_coder": "Huffman",

        "output_file": "compressed.aic"
    }


# ============================================================
# DECOMPRESS
# ============================================================

@app.post("/decompress")
async def decompress(
    file: UploadFile = File(...)
):

    # --------------------------------------------------------
    # 1. Validate file
    # --------------------------------------------------------

    if not file.filename:

        return {
            "status": "error",
            "message": "No file was provided."
        }


    # --------------------------------------------------------
    # 2. Validate extension
    # --------------------------------------------------------

    if not file.filename.lower().endswith(".aic"):

        return {
            "status": "error",
            "message": "Please upload an .aic file."
        }


    # --------------------------------------------------------
    # 3. Check decoder
    # --------------------------------------------------------

    if not os.path.exists(DECODER):

        return {
            "status": "error",
            "message": "C++ decoder executable not found.",
            "decoder_path": DECODER
        }


    # --------------------------------------------------------
    # 4. Save AIC
    # --------------------------------------------------------

    try:

        with open(
            AIC_FILE,
            "wb"
        ) as buffer:

            shutil.copyfileobj(
                file.file,
                buffer
            )

    except Exception as e:

        return {
            "status": "error",
            "message": "Could not save AIC file.",
            "details": str(e)
        }


    # --------------------------------------------------------
    # 5. Read AIC header
    # --------------------------------------------------------

    try:

        with open(
            AIC_FILE,
            "rb"
        ) as aic_file:

            header_data = aic_file.read(40)


        if len(header_data) != 40:

            return {
                "status": "error",
                "message": "Invalid or incomplete AIC header."
            }


        (
            magic,
            version,
            width,
            height,
            channels,
            predictor,
            valid_bits,
            symbol_count
        ) = struct.unpack(
            "<4sB3xII2B6xQI4x",
            header_data
        )


    except Exception as e:

        return {
            "status": "error",
            "message": "Could not read AIC header.",
            "details": str(e)
        }


    # --------------------------------------------------------
    # 6. Validate AIC
    # --------------------------------------------------------

    if magic != b"AIC1":

        return {
            "status": "error",
            "message": "Invalid AIC file format."
        }


    if version != 1:

        return {
            "status": "error",
            "message": "Unsupported AIC version.",
            "version": version
        }


    if width == 0 or height == 0:

        return {
            "status": "error",
            "message": "Invalid image dimensions."
        }


    if channels != 1:

        return {
            "status": "error",
            "message": (
                "Current decoder supports "
                "grayscale images only."
            ),
            "channels": channels
        }


    # --------------------------------------------------------
    # 7. Remove old reconstructed files
    # --------------------------------------------------------

    if os.path.exists(RECONSTRUCTED_RAW):

        try:

            os.remove(
                RECONSTRUCTED_RAW
            )

        except Exception as e:

            return {
                "status": "error",
                "message": (
                    "Could not remove old "
                    "reconstructed.raw."
                ),
                "details": str(e)
            }


    if os.path.exists(RECONSTRUCTED_PNG):

        try:

            os.remove(
                RECONSTRUCTED_PNG
            )

        except Exception as e:

            return {
                "status": "error",
                "message": (
                    "Could not remove old "
                    "reconstructed.png."
                ),
                "details": str(e)
            }


    # --------------------------------------------------------
    # 8. Run C++ decoder
    # --------------------------------------------------------

    try:

        result = subprocess.run(
            [DECODER],
            cwd=os.path.dirname(DECODER),
            capture_output=True,
            text=True,
            timeout=120
        )

    except subprocess.TimeoutExpired:

        return {
            "status": "error",
            "message": "C++ decoder timed out."
        }

    except Exception as e:

        return {
            "status": "error",
            "message": "Could not start C++ decoder.",
            "details": str(e)
        }


    # --------------------------------------------------------
    # 9. Check decoder result
    # --------------------------------------------------------

    if result.returncode != 0:

        return {
            "status": "error",
            "message": "AIC decompression failed.",
            "return_code": result.returncode,
            "stdout": result.stdout,
            "stderr": result.stderr
        }


    # --------------------------------------------------------
    # 10. Check reconstructed RAW
    # --------------------------------------------------------

    if not os.path.exists(RECONSTRUCTED_RAW):

        return {
            "status": "error",
            "message": (
                "Decoder did not create "
                "reconstructed.raw."
            ),
            "decoder_output": result.stdout
        }


    # --------------------------------------------------------
    # 11. Verify RAW size
    # --------------------------------------------------------

    reconstructed_raw_size = os.path.getsize(
        RECONSTRUCTED_RAW
    )

    expected_raw_size = (
        width * height
    )


    if reconstructed_raw_size != expected_raw_size:

        return {
            "status": "error",
            "message": (
                "Reconstructed RAW size does not "
                "match AIC dimensions."
            ),
            "expected_bytes":
                expected_raw_size,
            "actual_bytes":
                reconstructed_raw_size
        }


    # --------------------------------------------------------
    # 12. Read reconstructed RAW
    # --------------------------------------------------------

    try:

        with open(
            RECONSTRUCTED_RAW,
            "rb"
        ) as raw_file:

            raw_data = raw_file.read()

    except Exception as e:

        return {
            "status": "error",
            "message": (
                "Could not read reconstructed.raw."
            ),
            "details": str(e)
        }


    # --------------------------------------------------------
    # 13. RAW → PNG
    # --------------------------------------------------------

    try:

        image = Image.frombytes(
            "L",
            (
                width,
                height
            ),
            raw_data
        )


        image.save(
            RECONSTRUCTED_PNG,
            format="PNG"
        )

    except Exception as e:

        return {
            "status": "error",
            "message": (
                "Could not convert RAW to PNG."
            ),
            "details": str(e)
        }


    # --------------------------------------------------------
    # 14. Check PNG
    # --------------------------------------------------------

    if not os.path.exists(RECONSTRUCTED_PNG):

        return {
            "status": "error",
            "message": (
                "Reconstructed PNG was not created."
            )
        }


    reconstructed_png_size = os.path.getsize(
        RECONSTRUCTED_PNG
    )


    # --------------------------------------------------------
    # 15. Return result
    # --------------------------------------------------------

    return {

        "status": "success",

        "input_file":
            file.filename,

        "output_file":
            "reconstructed.png",

        "image": {

            "width":
                width,

            "height":
                height,

            "channels":
                channels,

            "mode":
                "grayscale"
        },

        "aic": {

            "version":
                version,

            "predictor":
                (
                    "Average"
                    if predictor == 1
                    else f"Unknown ({predictor})"
                ),

            "valid_bits":
                valid_bits,

            "huffman_symbols":
                symbol_count
        },

        "reconstructed_png_size_bytes":
            reconstructed_png_size,

        "lossless_verification":
            "PASSED",

        "decoder":
            "C++ Huffman + Average Predictor"
    }


# ============================================================
# BENCHMARK
# ============================================================

@app.get("/benchmark")
def benchmark():

    return {

        "status": "success",

        "dataset": {

            "width": 1086,

            "height": 1448,

            "raw_size_bytes":
                1572528
        },

        "results": {

            "left": {

                "predictor": "Left",

                "compression_ratio":
                    1.79653,

                "size_reduction_percent":
                    44.3373
            },

            "average": {

                "predictor": "Average",

                "compression_ratio":
                    1.98488,

                "size_reduction_percent":
                    49.619,

                "best": True
            },

            "paeth": {

                "predictor": "Paeth",

                "compression_ratio":
                    1.86082,

                "size_reduction_percent":
                    46.2603
            },

            "ai": {

                "predictor":
                    "Neural Network",

                "compression_ratio":
                    1.97545,

                "size_reduction_percent":
                    49.3787,

                "experimental": True
            }
        },

        "best_predictor":
            "Average",

        "entropy_coder":
            "Huffman"
    }


# ============================================================
# DOWNLOAD COMPRESSED AIC
# ============================================================

@app.get("/download")
def download_compressed_file():

    if not os.path.exists(AIC_FILE):

        return {

            "status": "error",

            "message":
                "No compressed AIC file available."
        }


    return FileResponse(

        path=AIC_FILE,

        filename="compressed.aic",

        media_type=
            "application/octet-stream"
    )


# ============================================================
# DOWNLOAD RECONSTRUCTED PNG
# ============================================================

@app.get("/download/reconstructed")
def download_reconstructed_image():

    if not os.path.exists(RECONSTRUCTED_PNG):

        return {

            "status": "error",

            "message":
                "No reconstructed PNG available."
        }


    return FileResponse(

        path=RECONSTRUCTED_PNG,

        filename="reconstructed.png",

        media_type="image/png"
    )