#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include <demo/demo.hpp>
#include <data/land_coords.hpp>

#define PI M_PI

demo::Vec3 spherical_to_3d(const demo::Vec2& s, float radius) {
    // s.x is longitude, s.y is latitude in (-pi/2, pi/2)
    // assumes x = 0, y = 0, z = +radius to be 0 deg, 0 deg
    return {
        cos(s.y) * radius * sin(s.x),
        -radius * sin(s.y),
        cos(s.y) * radius * cos(s.x)
    };
}

demo::Vec2 spherical_from_3d(const demo::Vec3& p) {
    float horiz_dist = std::sqrt(p.x * p.x + p.z * p.z);
    return {
        std::atan2(p.x, p.z),
        std::atan2(-p.y, horiz_dist)
    };
}

demo::Vec2 spherical_to_mercator(const demo::Vec2& s, float r) {
    return {
        r * s.x,
        -r * logf(tanf(.25f * PI + .5f * s.y))
    };
}

// 3x3 rotation matrix, row-major: p' = R * p
struct Mat3 {
    float m[3][3];
};

Mat3 mat3_identity() {
    return {{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} }};
}

Mat3 mat3_mul(const Mat3& a, const Mat3& b) {
    Mat3 r{};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}

Mat3 mat3_axis_angle(const demo::Vec3& axis, float angle) { // axis must be unit length
    // Rodrigues' rotation formula; same handedness as the Rx/Ry/Rz in rotate_globe
    const float c = cos(angle);
    const float s = sin(angle);
    const float t = 1 - c;
    const float x = axis.x, y = axis.y, z = axis.z;
    return {{
        { t*x*x + c,   t*x*y - s*z, t*x*z + s*y },
        { t*x*y + s*z, t*y*y + c,   t*y*z - s*x },
        { t*x*z - s*y, t*y*z + s*x, t*z*z + c   }
    }};
}

void mat3_orthonormalize(Mat3& r) {
    // Gram-Schmidt on the rows
    auto normalize = [](float* v) {
        const float n = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        v[0] /= n; v[1] /= n; v[2] /= n;
    };

    normalize(r.m[0]);

    const float d = r.m[1][0]*r.m[0][0] + r.m[1][1]*r.m[0][1] + r.m[1][2]*r.m[0][2];
    for (int i = 0; i < 3; i++)
        r.m[1][i] -= d * r.m[0][i];
    normalize(r.m[1]);

    // third row = row0 x row1 (keeps it a proper rotation, never a reflection)
    r.m[2][0] = r.m[0][1]*r.m[1][2] - r.m[0][2]*r.m[1][1];
    r.m[2][1] = r.m[0][2]*r.m[1][0] - r.m[0][0]*r.m[1][2];
    r.m[2][2] = r.m[0][0]*r.m[1][1] - r.m[0][1]*r.m[1][0];
}

class MercatorDemo : public demo::Demo {
private:
    const float zfactor = 0;

    demo::Vec3 globe_center;
    float globe_radius;

    std::vector<demo::Line3> sphere_og;
    std::vector<demo::Line3> sphere_tr;
    std::vector<demo::Line3> countries_og;
    std::vector<demo::Line3> countries_tr;

    float sphere_yaw = 0, sphere_pitch = 0, sphere_roll = 0;

    // Where the globe currently points: sphere_tr = orientation * sphere_og (+ center)
    Mat3 orientation = mat3_identity();

    const float spin_speed = 0.7f; // radians per second around the globe's own N-S axis

    bool was_held = false;
    demo::Vec2 last_pointer;

    void create_sphere_lines(float radius, int latitude_lines, int longitude_lines, int circle_sides) {
        globe_radius = radius;

        // latitude
        for (int i = 1; i <= latitude_lines; i++) {
            const float theta = i * PI / (latitude_lines + 1);
            const float circle_radius = radius * sin(theta);
            const float height = radius * cos(theta);
            for (int j = 0; j < circle_sides; j++) {
                const float alpha1 = 2 * j * PI / circle_sides;
                const float alpha2 = 2 * (j+1) * PI / circle_sides;
                demo::Line3 l;
                l.a.x = circle_radius * sin(alpha1);
                l.a.z = circle_radius * cos(alpha1);
                l.a.y = height;
                l.b.x = circle_radius * sin(alpha2);
                l.b.z = circle_radius * cos(alpha2);
                l.b.y = height;

                sphere_og.push_back(l);
            }
        }

        std::vector<demo::Line3> meridian;

        for (int j = 0; j < circle_sides; j++) { 
            const float alpha1 = 2 * j * PI / circle_sides;
            const float alpha2 = 2 * (j+1) * PI / circle_sides;
            demo::Line3 l;
            l.a.x = 0;
            l.a.y = radius * sin(alpha1);
            l.a.z = radius * cos(alpha1);
            l.b.x = 0;
            l.b.y = radius * sin(alpha2);
            l.b.z = radius * cos(alpha2);

            meridian.push_back(l);
        }

        for (int i = 0; i < longitude_lines; i++) {
            const float theta = i * PI / longitude_lines;
            for (const demo::Line3& l : meridian) {
                demo::Line3 rotated;
                rotated.a.x = cos(theta) * l.a.x + sin(theta) * l.a.z;
                rotated.a.y = l.a.y;
                rotated.a.z = -sin(theta) * l.a.x + cos(theta) * l.a.z;
                rotated.b.x = cos(theta) * l.b.x + sin(theta) * l.b.z;
                rotated.b.y = l.b.y;
                rotated.b.z = -sin(theta) * l.b.x + cos(theta) * l.b.z;

                sphere_og.push_back(rotated);
            }
        }

        for (const demo::Line3& l : sphere_og) {
            sphere_tr.push_back(demo::Line3(l));
        }
    }

    void create_country_lines() { // call once before transforming sphere
        for (const demo::Line2& sl : coordinate_data) {
            countries_og.push_back({
                spherical_to_3d(sl.a, globe_radius + 0),
                spherical_to_3d(sl.b, globe_radius + 0)
            });
        }

        for (demo::Line3& l : countries_og) {
            countries_tr.push_back(demo::Line3(l));
        }
    }
    
    void translate_globe(demo::Vec3 v) {
        globe_center.x += v.x;
        globe_center.y += v.y;
        globe_center.z += v.z;
        for (demo::Line3& l : sphere_tr) {
            l.a.x += v.x;
            l.a.y += v.y;
            l.a.z += v.z;
            l.b.x += v.x;
            l.b.y += v.y;
            l.b.z += v.z;
        }

        for (demo::Line3& l : countries_tr) {
            l.a.x += v.x;
            l.a.y += v.y;
            l.a.z += v.z;
            l.b.x += v.x;
            l.b.y += v.y;
            l.b.z += v.z;
        }
    }

    void unrotate_globe() {
        demo::Vec3 temp_center = globe_center;
        globe_center = {0.0, 0.0, 0.0};

        for (auto i = 0; i < sphere_og.size(); i++) {
            sphere_tr[i].a = sphere_og[i].a;
            sphere_tr[i].b = sphere_og[i].b;
        }

        for (auto i = 0; i < countries_og.size(); i++) {
            countries_tr[i].a = countries_og[i].a;
            countries_tr[i].b = countries_og[i].b;
        }

        translate_globe(temp_center);
    }

    void rotate_globe(float alpha, float beta, float gamma) {
        // rotate all the lines in sphere_lines about (0, 0, 0)
        // alpha = rotation around X
        // beta  = rotation around Y
        // gamma = rotation around Z

        const float ca = cos(alpha);
        const float sa = sin(alpha);
        const float cb = cos(beta);
        const float sb = sin(beta);
        const float cg = cos(gamma);
        const float sg = sin(gamma);

        // R = Rz * Ry * Rx
        const float r00 = cg * cb;
        const float r01 = cg * sb * sa - sg * ca;
        const float r02 = cg * sb * ca + sg * sa;

        const float r10 = sg * cb;
        const float r11 = sg * sb * sa + cg * ca;
        const float r12 = sg * sb * ca - cg * sa;

        const float r20 = -sb;
        const float r21 = cb * sa;
        const float r22 = cb * ca;

        auto rotate = [&](demo::Vec3& p) {
            const float x = p.x;
            const float y = p.y;
            const float z = p.z;

            p.x = r00 * x + r01 * y + r02 * z;
            p.y = r10 * x + r11 * y + r12 * z;
            p.z = r20 * x + r21 * y + r22 * z;
        };

        demo::Vec3 temp_center = globe_center;

        translate_globe({-globe_center.x, -globe_center.y, -globe_center.z});

        for (demo::Line3& l : sphere_tr) {
            rotate(l.a);
            rotate(l.b);
        }

        for (demo::Line3& l : countries_tr) {
            rotate(l.a);
            rotate(l.b);
        }

        translate_globe(temp_center);
    }

    // same as above, but takes the rotation as a ready-made matrix
    void rotate_globe(const Mat3& r) {
        auto rotate = [&](demo::Vec3& p) {
            const float x = p.x;
            const float y = p.y;
            const float z = p.z;

            p.x = r.m[0][0] * x + r.m[0][1] * y + r.m[0][2] * z;
            p.y = r.m[1][0] * x + r.m[1][1] * y + r.m[1][2] * z;
            p.z = r.m[2][0] * x + r.m[2][1] * y + r.m[2][2] * z;
        };

        demo::Vec3 temp_center = globe_center;

        translate_globe({-globe_center.x, -globe_center.y, -globe_center.z});

        for (demo::Line3& l : sphere_tr) {
            rotate(l.a);
            rotate(l.b);
        }

        for (demo::Line3& l : countries_tr) {
            rotate(l.a);
            rotate(l.b);
        }

        translate_globe(temp_center);
    }

    // Rotate the globe about a fixed SCREEN axis (applied after the current orientation).
    // This is what dragging uses: the axis doesn't depend on how the globe is currently tilted.
    void rotate_orientation_screen(const demo::Vec3& axis, float angle) {
        orientation = mat3_mul(mat3_axis_angle(axis, angle), orientation);
    }

    // Rotate the globe about its OWN N-S axis (applied before the current orientation).
    // This is the slow "Earth spinning" motion.
    void spin_orientation(float angle) {
        orientation = mat3_mul(orientation, mat3_axis_angle({0, 1, 0}, angle));
    }

    bool drag_globe(const demo::InputData& input) {
        if (input.pointer_held && was_held) {
            const float dx = input.pointer.x - last_pointer.x;
            const float dy = input.pointer.y - last_pointer.y;

            // 1 pixel of drag = 1/radius radians, so a point at the middle of the globe
            // follows the pointer. Dragging right turns the near side right (about screen Y);
            // dragging down turns it down (about screen X, hence the minus).
            const float wx = -dy / globe_radius;
            const float wy =  dx / globe_radius;
            const float angle = std::sqrt(wx * wx + wy * wy);

            if (angle > 1e-6f)
                rotate_orientation_screen({ wx / angle, wy / angle, 0 }, angle);
        }

        was_held = input.pointer_held;
        last_pointer = input.pointer;

        return input.pointer_held;
    }

    void draw_line3(demo::Canvas& canvas, const demo::Line3& line) {
        // add perspective later
        
        canvas.line({
                        line.a.x,
                        line.a.y
                    }, {
                        line.b.x, 
                        line.b.y
                    });
    }
    
    void draw_globe(demo::Canvas& canvas, bool seethrough) {
        canvas.stroke({ 80, 80, 100, 255 });
        canvas.fill({180, 180, 200, 255});
        if (!seethrough)
            canvas.circle({globe_center.x, globe_center.y}, globe_radius);

        std::vector<demo::Vec2> coords;
        
        for (demo::Line3& l : sphere_tr) {
            if (seethrough || l.a.z >= 0 || l.b.z >= 0) {
                coords.push_back({ l.a.x, l.a.y });
                coords.push_back({ l.b.x, l.b.y });
            }
        }
        
        canvas.stroke({ 150, 150, 160, 255 });
        canvas.polyline(coords);

        coords.clear();

        for (demo::Line3& l : countries_tr) {
            if (seethrough || l.a.z >= 0 || l.b.z >= 0) {
                coords.push_back({ l.a.x, l.a.y });
                coords.push_back({ l.b.x, l.b.y });
            }
        }
        
        canvas.stroke({ 0, 0, 0, 255 });
        canvas.polyline(coords);
    }

    void draw_mercator(demo::Canvas& canvas, demo::Vec2 topleft, float r) {
        std::vector<demo::Vec2> coords;
        for (demo::Line3& l : sphere_tr) {
            demo::Vec2 s_a = spherical_from_3d(l.a - globe_center);
            demo::Vec2 s_b = spherical_from_3d(l.b - globe_center);

            demo::Vec2 a = spherical_to_mercator(s_a, r);
            demo::Vec2 b = spherical_to_mercator(s_b, r);

            if (fabs(a.x - b.x) > r)
                continue;

            coords.push_back(topleft + a);
            coords.push_back(topleft + b);
        }

        canvas.stroke({ 150, 150, 160, 255 });
        canvas.polyline(coords);

        coords.clear();

        for (demo::Line3& l : countries_tr) {
            demo::Vec2 s_a = spherical_from_3d(l.a - globe_center);
            demo::Vec2 s_b = spherical_from_3d(l.b - globe_center);

            demo::Vec2 a = spherical_to_mercator(s_a, r);
            demo::Vec2 b = spherical_to_mercator(s_b, r);

            if (fabs(a.x - b.x) > r)
                continue;

            coords.push_back(topleft + a);
            coords.push_back(topleft + b);
        }

        canvas.stroke({ 0, 0, 0, 255 });
        canvas.polyline(coords);        
    }

public:
    const char* title() const override { return "Earth demo"; };
    demo::Vec2 preferred_size() const override { return {820, 400}; }

    MercatorDemo() {
        create_sphere_lines(preferred_size().y * 0.45f, 13, 13, 100);
        create_country_lines();
        translate_globe({preferred_size().y / 2, preferred_size().y / 2, 0.0});

        // rotate_orientation_screen({1, 0, 0}, -0.4f); // starting tilt: north pole leans toward the viewer
        unrotate_globe();
        rotate_globe(orientation);
    }

    void update(float dt, const demo::InputData& input) override {
        const bool dragging = drag_globe(input);

        if (!dragging) { // don't fight the user while they hold the globe
            // spin_orientation(spin_speed * dt);
            rotate_orientation_screen({0.0, 1.0, 0.0}, spin_speed * dt);
        }

        mat3_orthonormalize(orientation);

        unrotate_globe();
        rotate_globe(orientation);
    }

    void draw(demo::Canvas& canvas) override {
        canvas.fill({180, 180, 200, 255});
        canvas.clear();
        draw_globe(canvas, false);
        draw_mercator(canvas, {preferred_size().x * .75f, preferred_size().y * .5f}, preferred_size().y * 0.55f / PI);
    }
};

std::unique_ptr<demo::Demo> demo::create_demo() {
    return std::make_unique<MercatorDemo>();
}