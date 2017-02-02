# Optimized-Mandelbrot
An Optimized (simple) Mandelbrot Renderer

This is a simple Mandelbrot renderer that I wrote a couple of years ago that I have infrequently and slowly optimized using SIMD, assembly, and multi-threading techniques since I originally wrote it. The emphasis for this project was to learn optimization and little effort has been put in other areas.

How to use:
* Grab the files by clicking "Clone or download"->"Download Zip" and unzip
* Run main.exe that is found in the build folder
* Use arrow keys to move around; + and - keys to zoom in and out. 

---

I plan on doing at least the following at some point:
* Clean up the code (sorry for the mess!)
* Add Color
* Infinite Mandelbrot. Currently the depth of the zoom is limited by 64-bit floating point precision. I would like to add infinite zoom/precision rendering (as soon as I get around to finishing my infinite precision floating point implementation...).
* Proper, optimized hardware acceleration. I did a simple GPU version of the mandelbrot renderer a while ago but I would like to do a proper one that is optimized in the future. I would like to see if I can do a combined CPU and GPU render as well.

---

The following is pseudo-analysis of results (before synchronized rendering was added) on a processor with 12 logical cores (Intel Core i7-5820K Haswell-E 6-Core 3.3GHz):

Approximate number of clock cycles to compute one frame of the initial view:
* Pre-optimization: ~2.2 billion clock cycles
* Post-optimization: ~10 million clock cycles (per logical cpu core)

Translation to per-frame timing:
* Pre-optimization: 670 milliseconds
* Post-optimization: 3 milliseconds

---

Resources:
* [MSDN](https://msdn.microsoft.com/en-us)
* [Intel Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/)
* [Handmade Hero](https://www.youtube.com/channel/UCaTznQhurW5AaiYPbhEA-KA)
* [Introduction to x64 Assembly](https://software.intel.com/en-us/articles/introduction-to-x64-assembly)
