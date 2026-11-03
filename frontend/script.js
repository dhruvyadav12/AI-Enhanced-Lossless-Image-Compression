// ============================================================
// AI LOSSLESS IMAGE COMPRESSOR - FRONTEND
// ============================================================

const API_URL = (window.location.protocol === "http:" || window.location.protocol === "https:")
    ? window.location.origin
    : "http://127.0.0.1:8000";


// ============================================================
// PAGE INITIALIZATION
// ============================================================

document.addEventListener("DOMContentLoaded", function () {

    console.log("AI Lossless frontend loaded");


    // ========================================================
    // ELEMENTS
    // ========================================================

    const imageInput =
        document.getElementById("imageInput");

    const aicInput =
        document.getElementById("aicInput");

    const selectedImageName =
        document.getElementById("selectedImageName");

    const selectedAicName =
        document.getElementById("selectedAicName");

    const compressButton =
        document.getElementById("compressButton");

    const decompressButton =
        document.getElementById("decompressButton");

    const compressionResult =
        document.getElementById("compressionResult");

    const decompressionResult =
        document.getElementById("decompressionResult");

    const compressionError =
        document.getElementById("compressionError");

    const decompressionError =
        document.getElementById("decompressionError");

    const downloadAicButton =
        document.getElementById("downloadAicButton");

    const downloadPngButton =
        document.getElementById("downloadPngButton");


    // ========================================================
    // DEBUG - CHECK ELEMENTS
    // ========================================================

    console.log("Image input:", imageInput);
    console.log("Compress button:", compressButton);
    console.log("AIC input:", aicInput);
    console.log("Decompress button:", decompressButton);


    if (!imageInput) {
        console.error("ERROR: imageInput not found.");
    }

    if (!compressButton) {
        console.error("ERROR: compressButton not found.");
    }


    // ========================================================
    // FORMAT BYTES
    // ========================================================

    function formatBytes(bytes) {

        if (
            bytes === undefined ||
            bytes === null ||
            bytes === 0
        ) {
            return "0 Bytes";
        }


        const units = [
            "Bytes",
            "KB",
            "MB",
            "GB"
        ];


        const index = Math.floor(
            Math.log(bytes) /
            Math.log(1024)
        );


        return (
            (bytes / Math.pow(1024, index))
                .toFixed(2)
            + " "
            + units[index]
        );
    }


    // ========================================================
    // SHOW ERROR
    // ========================================================

    function showError(element, message) {

        if (!element) {
            return;
        }

        element.textContent = message;
        element.hidden = false;
    }


    // ========================================================
    // HIDE ERROR
    // ========================================================

    function hideError(element) {

        if (!element) {
            return;
        }

        element.textContent = "";
        element.hidden = true;
    }


    // ========================================================
    // IMAGE SELECTION
    // ========================================================

    if (imageInput) {

        imageInput.addEventListener(
            "change",
            function () {

                console.log(
                    "Image selection changed."
                );


                if (
                    !this.files ||
                    this.files.length === 0
                ) {

                    selectedImageName.textContent = "";

                    return;
                }


                const file = this.files[0];


                console.log(
                    "Selected image:",
                    file.name
                );

                console.log(
                    "Image type:",
                    file.type
                );

                console.log(
                    "Image size:",
                    file.size,
                    "bytes"
                );


                if (selectedImageName) {

                    selectedImageName.textContent =
                        `Selected: ${file.name}`;
                }
            }
        );
    }


    // ========================================================
    // AIC FILE SELECTION
    // ========================================================

    if (aicInput) {

        aicInput.addEventListener(
            "change",
            function () {

                console.log(
                    "AIC selection changed."
                );


                if (
                    !this.files ||
                    this.files.length === 0
                ) {

                    if (selectedAicName) {
                        selectedAicName.textContent = "";
                    }

                    return;
                }


                const file = this.files[0];


                console.log(
                    "Selected AIC:",
                    file.name
                );


                if (selectedAicName) {

                    selectedAicName.textContent =
                        `Selected: ${file.name}`;
                }
            }
        );
    }


    // ========================================================
    // COMPRESS IMAGE
    // ========================================================

    if (compressButton) {

        compressButton.addEventListener(
            "click",
            async function (event) {

                event.preventDefault();


                console.log(
                    "================================"
                );

                console.log(
                    "COMPRESS BUTTON CLICKED"
                );

                console.log(
                    "================================"
                );


                hideError(
                    compressionError
                );


                if (compressionResult) {

                    compressionResult.hidden = true;
                }


                // ------------------------------------------------
                // GET CURRENT INPUT DIRECTLY
                // ------------------------------------------------

                const currentInput =
                    document.getElementById(
                        "imageInput"
                    );


                if (!currentInput) {

                    showError(
                        compressionError,
                        "Image input element not found."
                    );

                    console.error(
                        "imageInput element not found."
                    );

                    return;
                }


                // ------------------------------------------------
                // CHECK FILE
                // ------------------------------------------------

                console.log(
                    "Files in input:",
                    currentInput.files.length
                );


                if (
                    !currentInput.files ||
                    currentInput.files.length === 0
                ) {

                    console.error(
                        "No image selected."
                    );


                    showError(
                        compressionError,
                        "Please choose an image first."
                    );

                    return;
                }


                // ------------------------------------------------
                // GET FILE
                // ------------------------------------------------

                const file =
                    currentInput.files[0];


                console.log(
                    "File selected:",
                    file.name
                );

                console.log(
                    "File type:",
                    file.type
                );

                console.log(
                    "File size:",
                    file.size,
                    "bytes"
                );


                // ------------------------------------------------
                // CREATE FORMDATA
                // ------------------------------------------------

                const formData =
                    new FormData();


                formData.append(
                    "file",
                    file
                );


                console.log(
                    "FormData created."
                );


                // ------------------------------------------------
                // BUTTON STATE
                // ------------------------------------------------

                compressButton.disabled = true;

                compressButton.textContent =
                    "Compressing...";


                try {

                    console.log(
                        "Sending POST request..."
                    );

                    console.log(
                        "URL:",
                        `${API_URL}/compress`
                    );


                    // ------------------------------------------------
                    // API REQUEST
                    // ------------------------------------------------

                    const response =
                        await fetch(
                            `${API_URL}/compress`,
                            {
                                method: "POST",
                                body: formData
                            }
                        );


                    console.log(
                        "Response received."
                    );

                    console.log(
                        "HTTP status:",
                        response.status
                    );


                    // ------------------------------------------------
                    // READ RESPONSE
                    // ------------------------------------------------

                    const data =
                        await response.json();


                    console.log(
                        "Compression response:",
                        data
                    );


                    // ------------------------------------------------
                    // CHECK RESPONSE
                    // ------------------------------------------------

                    if (
                        !response.ok ||
                        data.status !== "success"
                    ) {

                        throw new Error(
                            data.message ||
                            data.error ||
                            "Compression failed."
                        );
                    }


                    // ------------------------------------------------
                    // GET COMPRESSION DATA
                    // ------------------------------------------------

                    const compression =
                        data.compression;


                    if (!compression) {

                        throw new Error(
                            "Compression data missing from API response."
                        );
                    }


                    // ------------------------------------------------
                    // ORIGINAL SIZE
                    // ------------------------------------------------

                    const originalSize =
                        document.getElementById(
                            "originalSize"
                        );


                    if (originalSize) {

                        originalSize.textContent =
                            formatBytes(
                                compression
                                    .original_size_bytes
                            );
                    }


                    // ------------------------------------------------
                    // COMPRESSED SIZE
                    // ------------------------------------------------

                    const compressedSize =
                        document.getElementById(
                            "compressedSize"
                        );


                    if (compressedSize) {

                        compressedSize.textContent =
                            formatBytes(
                                compression
                                    .compressed_size_bytes
                            );
                    }


                    // ------------------------------------------------
                    // COMPRESSION RATIO
                    // ------------------------------------------------

                    const compressionRatio =
                        document.getElementById(
                            "compressionRatio"
                        );


                    if (compressionRatio) {

                        compressionRatio.textContent =
                            compression
                                .compression_ratio
                            + "×";
                    }


                    // ------------------------------------------------
                    // SIZE REDUCTION
                    // ------------------------------------------------

                    const sizeReduction =
                        document.getElementById(
                            "sizeReduction"
                        );


                    if (sizeReduction) {

                        sizeReduction.textContent =
                            compression
                                .size_reduction_percent
                            + "%";
                    }


                    // ------------------------------------------------
                    // SHOW RESULT
                    // ------------------------------------------------

                    if (compressionResult) {

                        compressionResult.hidden =
                            false;
                    }


                    console.log(
                        "================================"
                    );

                    console.log(
                        "COMPRESSION SUCCESSFUL"
                    );

                    console.log(
                        "================================"
                    );

                }
                catch (error) {

                    console.error(
                        "COMPRESSION ERROR:",
                        error
                    );


                    showError(
                        compressionError,
                        error.message
                    );

                }
                finally {

                    compressButton.disabled =
                        false;

                    compressButton.textContent =
                        "Compress Image";
                }

            }
        );
    }


    // ========================================================
    // DOWNLOAD AIC
    // ========================================================

    if (downloadAicButton) {

        downloadAicButton.addEventListener(
            "click",
            function () {

                console.log(
                    "Downloading compressed.aic"
                );


                window.location.href =
                    `${API_URL}/download`;
            }
        );
    }


    // ========================================================
    // DECOMPRESS AIC
    // ========================================================

    if (decompressButton) {

        decompressButton.addEventListener(
            "click",
            async function (event) {

                event.preventDefault();


                console.log(
                    "================================"
                );

                console.log(
                    "DECOMPRESS BUTTON CLICKED"
                );

                console.log(
                    "================================"
                );


                hideError(
                    decompressionError
                );


                if (decompressionResult) {

                    decompressionResult.hidden =
                        true;
                }


                // ------------------------------------------------
                // GET INPUT DIRECTLY
                // ------------------------------------------------

                const currentAicInput =
                    document.getElementById(
                        "aicInput"
                    );


                if (!currentAicInput) {

                    showError(
                        decompressionError,
                        "AIC input element not found."
                    );

                    return;
                }


                // ------------------------------------------------
                // CHECK FILE
                // ------------------------------------------------

                if (
                    !currentAicInput.files ||
                    currentAicInput.files.length === 0
                ) {

                    showError(
                        decompressionError,
                        "Please choose an .aic file first."
                    );

                    return;
                }


                const file =
                    currentAicInput.files[0];


                console.log(
                    "AIC file:",
                    file.name
                );

                console.log(
                    "AIC size:",
                    file.size
                );


                // ------------------------------------------------
                // VALIDATE EXTENSION
                // ------------------------------------------------

                if (
                    !file.name
                        .toLowerCase()
                        .endsWith(".aic")
                ) {

                    showError(
                        decompressionError,
                        "Please select a valid .aic file."
                    );

                    return;
                }


                // ------------------------------------------------
                // CREATE FORMDATA
                // ------------------------------------------------

                const formData =
                    new FormData();


                formData.append(
                    "file",
                    file
                );


                decompressButton.disabled =
                    true;

                decompressButton.textContent =
                    "Decompressing...";


                try {

                    console.log(
                        "Sending decompression request..."
                    );


                    const response =
                        await fetch(
                            `${API_URL}/decompress`,
                            {
                                method: "POST",
                                body: formData
                            }
                        );


                    console.log(
                        "Decompression status:",
                        response.status
                    );


                    const data =
                        await response.json();


                    console.log(
                        "Decompression response:",
                        data
                    );


                    if (
                        !response.ok ||
                        data.status !== "success"
                    ) {

                        throw new Error(
                            data.message ||
                            data.error ||
                            "Decompression failed."
                        );
                    }


                    // ------------------------------------------------
                    // SHOW RESULT
                    // ------------------------------------------------

                    if (decompressionResult) {

                        decompressionResult.hidden =
                            false;
                    }


                    console.log(
                        "================================"
                    );

                    console.log(
                        "DECOMPRESSION SUCCESSFUL"
                    );

                    console.log(
                        "================================"
                    );

                }
                catch (error) {

                    console.error(
                        "DECOMPRESSION ERROR:",
                        error
                    );


                    showError(
                        decompressionError,
                        error.message
                    );

                }
                finally {

                    decompressButton.disabled =
                        false;

                    decompressButton.textContent =
                        "Decompress AIC";
                }

            }
        );
    }


    // ========================================================
    // DOWNLOAD RECONSTRUCTED PNG
    // ========================================================

    if (downloadPngButton) {

        downloadPngButton.addEventListener(
            "click",
            function () {

                console.log(
                    "Downloading reconstructed.png"
                );


                window.location.href =
                    `${API_URL}/download/reconstructed`;
            }
        );
    }


    // ========================================================
    // BENCHMARK
    // ========================================================

    async function loadBenchmark() {

        const container =
            document.getElementById(
                "benchmarkContainer"
            );


        if (!container) {

            console.error(
                "Benchmark container not found."
            );

            return;
        }


        try {

            console.log(
                "Loading benchmark..."
            );


            const response =
                await fetch(
                    `${API_URL}/benchmark`
                );


            console.log(
                "Benchmark HTTP status:",
                response.status
            );


            if (!response.ok) {

                throw new Error(
                    `Benchmark HTTP error: ${response.status}`
                );
            }


            const data =
                await response.json();


            console.log(
                "Benchmark data:",
                data
            );


            if (data.status !== "success") {

                throw new Error(
                    "Benchmark unavailable."
                );
            }


            const results =
                data.results;


            const maxRatio =
                Math.max(
                    ...Object.values(results)
                        .map(
                            item =>
                                item.compression_ratio
                        )
                );


            const order = [
                "average",
                "ai",
                "paeth",
                "left"
            ];


            container.innerHTML = "";


            order.forEach(
                function (key) {

                    const item =
                        results[key];


                    if (!item) {
                        return;
                    }


                    const percentage =
                        (
                            item.compression_ratio /
                            maxRatio
                        ) * 100;


                    const row =
                        document.createElement(
                            "div"
                        );


                    row.className =
                        "benchmark-row";


                    const bestLabel =
                        item.best
                            ? `<span class="best">BEST</span>`
                            : "";


                    row.innerHTML = `

                        <div class="predictor-name">

                            ${item.predictor}

                            ${bestLabel}

                        </div>

                        <div class="bar-container">

                            <div
                                class="bar"
                                style="width: ${percentage}%"
                            ></div>

                        </div>

                        <div class="ratio">

                            ${item.compression_ratio}×

                        </div>

                    `;


                    container.appendChild(
                        row
                    );

                }
            );


            console.log(
                "Benchmark loaded successfully."
            );

        }
        catch (error) {

            console.error(
                "BENCHMARK ERROR:",
                error
            );


            container.textContent =
                "Could not load benchmark.";
        }
    }


    // ========================================================
    // LOAD BENCHMARK
    // ========================================================

    loadBenchmark();


    // ========================================================
    // INITIALIZATION COMPLETE
    // ========================================================

    console.log(
        "Frontend initialization complete."
    );

});