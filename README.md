# C++ Ray Tracer

A CPU-based ray tracer built in C++ while working through *Ray Tracing in One Weekend*. This project explores ray intersections, camera sampling, and diffuse light scattering.

## Progress

![Ray tracer progress render](image.png)

The renderer currently includes:

- Sphere intersections with closest-hit selection across a scene.
- Surface normals oriented against the incoming ray.
- A camera class that handles image setup, ray generation, and rendering.
- Anti-aliasing through random samples within each pixel.
- Recursive diffuse scattering with 50% attenuation at each bounce.
- A sky gradient and plain-text PPM image output.

The default scene contains a small sphere above a large ground sphere. It renders at **400 × 225 pixels** with **50 samples per pixel**.

## Build and Run

You need a C++17 compiler such as Clang or GCC. There are no external library dependencies.

Change into your `ray-tacer` checkout first (running from another project such as `accrual-app` will fail to find `srcs/main`):

```sh
cd /path/to/ray-tacer
mkdir -p build
clang++ -std=c++17 -O2 -x c++ srcs/main -o build/ray-tacer
./build/ray-tacer > image.ppm
```

You can substitute `g++` for `clang++`. The `-x c++` flag is needed because `srcs/main` has no file extension.

Image data goes to standard output, while scanline progress goes to standard error. The generated `image.ppm` can be opened in a PPM-compatible image viewer. On macOS:

```sh
open image.ppm
```

## Configure the Render

Edit the camera settings in `srcs/main` before calling `render()`:

```cpp
camera cam;
cam.aspect_ratio = 16.0 / 9.0;
cam.image_width = 400;
cam.samples_per_pix = 50;
cam.render(world);
```

Use a positive image width and aspect ratio, and a positive whole number of samples. Image height is calculated from the width and aspect ratio. More samples reduce noise and smooth edges, but take longer to render.

Add spheres to `world` before rendering:

```cpp
world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));
```

## How It Works

A ray is represented by `P(t) = O + tD`, where `O` is its origin, `D` is its direction, and `t` is a parameter along the ray. The camera sends rays through randomly selected positions within each pixel.

For each ray, the scene finds the closest sphere intersection within the allowed interval. At a hit, the renderer chooses a diffuse direction using the surface normal plus a random unit vector, traces another ray, and halves the returned colour. Rays that miss the scene return the sky gradient.

The camera averages all samples for a pixel. The colour writer clamps the result and converts it to integer RGB values for the PPM file.

## Project Structure

```text
srcs/
├── main             # Scene construction and camera settings
├── camera.h         # Camera setup, sampling, and recursive rendering
├── hittable.h       # Intersection interface and hit records
├── hittable_list.h  # Scene objects and closest-hit selection
├── sphere.h         # Sphere intersection implementation
├── interval.h       # Numeric bounds and clamping
├── ray.h            # Ray origin, direction, and points along a ray
├── vec3.h           # Vector math and random direction helpers
├── colour.h         # RGB conversion and PPM pixel output
└── utils.h          # Shared constants, random numbers, and includes
```

## Work in Progress

Diffuse scattering is implemented directly in the camera's shading function. Separate materials, metallic reflection, dielectric refraction, depth of field, and gamma correction are not implemented yet.

Recursive scattering is limited by `cam.max_depth` (50 by default). Secondary intersections use a small positive lower bound to avoid self-intersections.
