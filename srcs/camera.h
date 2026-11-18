#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "utils.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

struct render_stats {
    double seconds;
    unsigned threads;
    int width;
    int height;
};

class camera {
public:
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int samples_per_pix = 50;
    int max_depth = 50;
    unsigned thread_count = 1;
    std::uint64_t seed = 2027;

    render_stats render(const hittable& world, std::ostream& out = std::cout,
                        bool write_image = true) {
        if (image_width < 1 || image_width > 32768 || samples_per_pix < 1 ||
            max_depth < 1 || max_depth > 256 || thread_count < 1 ||
            !std::isfinite(aspect_ratio) || aspect_ratio <= 0 ||
            image_width / aspect_ratio > 32768) {
            throw std::invalid_argument("invalid camera settings");
        }
        const auto start = std::chrono::steady_clock::now();
        initialize();
        std::vector<colour> pixels(static_cast<std::size_t>(image_width) * image_height);
        const auto workers = std::min(thread_count, static_cast<unsigned>(image_height));
        std::atomic<int> next_row{0};
        std::atomic<bool> stop{false};
        std::mutex failure_mutex;
        std::exception_ptr failure;
        // Rows vary in cost (sky versus diffuse bounces), so claim them dynamically.
        auto worker = [&] {
            try {
                while (!stop.load(std::memory_order_relaxed)) {
                    const int j = next_row.fetch_add(1, std::memory_order_relaxed);
                    if (j >= image_height) break;
                    for (int i = 0; i < image_width; ++i) {
                        const auto index = static_cast<std::size_t>(j) * image_width + i;
                        // Pixel identity, not worker identity, selects its random stream.
                        seed_random(seed + UINT64_C(0x9e3779b97f4a7c15) * (index + 1));
                        colour sum(0, 0, 0);
                        for (int sample = 0; sample < samples_per_pix; ++sample)
                            sum += ray_colour(get_ray(i, j), max_depth, world);
                        pixels[index] = sample_scale * sum;
                    }
                }
            } catch (...) {
                std::lock_guard<std::mutex> lock(failure_mutex);
                if (!failure) failure = std::current_exception();
                stop.store(true, std::memory_order_relaxed);
            }
        };
        if (workers == 1) {
            worker();
        } else {
            std::vector<std::thread> threads;
            threads.reserve(workers);
            try {
                for (unsigned t = 0; t < workers; ++t) threads.emplace_back(worker);
            } catch (...) {
                stop.store(true, std::memory_order_relaxed);
                for (auto& t : threads) t.join();
                throw;
            }
            for (auto& t : threads) t.join();
        }
        if (failure) std::rethrow_exception(failure);
        const double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start).count();
        // Only the caller writes the image, in pixel order, after joining.
        if (write_image) {
            out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
            for (const auto& pixel : pixels) write_colour(out, pixel);
        }
        return {elapsed, workers, image_width, image_height};
    }

private:
    int image_height;
    point3 camera_center;
    point3 p00_center;
    vec3 pixel_u;
    vec3 pixel_v;
    double sample_scale;

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(image_width) / image_height);
        camera_center = point3(0, 0, 0);

        sample_scale = 1.0/samples_per_pix;

        // Vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        // Horizontal and vertical offsets between pixels.
        pixel_u = viewport_u / image_width;
        pixel_v = viewport_v / image_height;

        auto upper_left = camera_center
                        - vec3(0, 0, focal_length)
                        - viewport_u / 2
                        - viewport_v / 2;
        p00_center = upper_left + 0.5 * (pixel_u + pixel_v);
    }

    ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the origin and directed at randomly sampled
        // point around the pixel location i, j
        auto offset = sample();
        auto pixel_sample = p00_center + ((i+offset.x()) * pixel_u) + ((j+offset.y()) * pixel_v);
        auto ray_origin =camera_center;
        auto ray_direction = pixel_sample - camera_center;

        return ray(ray_origin, ray_direction);

    }

    vec3 sample() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);

    }

    colour ray_colour(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0) return colour(0, 0, 0);

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec)) {
            vec3 direction = rec.normal+random_unit_vector();
            if (direction.near_zero()) direction = rec.normal;
            return 0.5 * ray_colour(ray(rec.p, direction), depth-1, world);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * colour(1.0, 1.0, 1.0)
            + a * colour(0.5, 0.7, 1.0);
    }
};

#endif
