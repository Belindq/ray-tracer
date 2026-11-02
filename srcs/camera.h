#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "utils.h"
class camera {
public:
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    double samples_per_pix=100;

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << image_height - j << ' '
                      << std::flush;
            for (int i = 0; i < image_width; i++) {
                colour colour_pixel(0,0,0);
                for (int sample =0; sample<samples_per_pix;sample++){
                    ray r = get_ray(i,j);
                    colour_pixel +=ray_colour(r, world);
                }   
                write_colour(std::cout, sample_scale * colour_pixel);

            }
        }
        std::clog << "\rDone.                 \n";
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

    ray get_ray(int i, int j){
        // Construct a camera ray originating from the origin and directed at randomly sampled
        // point around the pixel location i, j
        auto offset = sample();
        auto pixel_sample = p00_center + ((i+offset.x()) * pixel_u) + ((j+offset.y()) * pixel_v);
        auto ray_origin =camera_center;
        auto ray_direction = pixel_sample - camera_center;

        return ray(ray_origin, ray_direction);

    }

    vec3 sample(){
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);

    }

    colour ray_colour(const ray& r, const hittable& world) const {
        hit_record rec;
        if (world.hit(r, interval(0, infinity), rec)) {
            return 0.5 * (rec.normal + colour(1, 1, 1));
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * colour(1.0, 1.0, 1.0)
            + a * colour(0.5, 0.7, 1.0);
    }
};

#endif
