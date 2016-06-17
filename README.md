# Optimized-Mandelbrot
An Optimized (simple) Mandelbrot Renderer

This is a simple Mandelbrot renderer that I wrote a couple of years ago that I have infrequently and slowly optimized using SIMD, assembly, and multi-threading techniques since I originally wrote it. The emphasis for this project was to learn optimization and little effort has been put in other areas.

As of this writing I plan on doing at least the following at some point:
* Clean up the code (sorry for the mess!)
* Improve the multi-threading code, including fixing the "tearing" issue (horizontal line artifacts that appear as the user navigates) that is caused by asynchronous execution of worker threads.

---

The following is pseudo-analysis of results on a processor with 12 logical cores (Intel Core i7-5820K Haswell-E 6-Core 3.3GHz):

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
