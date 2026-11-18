#include "camera.h"
#include <cassert>
#include <sstream>

class failing_scene : public hittable {
public:
    bool hit(const ray&, interval, hit_record&) const override {
        throw std::runtime_error("scene failure");
    }
};

int main() {
    // Worker exceptions must join all peers and reach the caller, not terminate.
    for (unsigned threads : {1u, 4u}) {
        camera cam;
        cam.image_width = 16;
        cam.samples_per_pix = 1;
        cam.thread_count = threads;
        std::ostringstream output;
        bool caught = false;
        try {
            cam.render(failing_scene{}, output);
        } catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "scene failure";
        }
        assert(caught);
        assert(output.str().empty());
    }
}
