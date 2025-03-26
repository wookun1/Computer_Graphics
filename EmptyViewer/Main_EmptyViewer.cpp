#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>
#include <cmath>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glm;

// --------------------------
// 기본 클래스 정의
// --------------------------

// Ray: 광선의 원점과 정규화된 방향
class Ray {
public:
    vec3 origin;
    vec3 direction;
    Ray(const vec3& o, const vec3& d) : origin(o), direction(normalize(d)) { }
};

// Surface: 모든 장면 객체가 상속받을 추상 클래스
class Surface {
public:
    virtual ~Surface() {}
    // 주어진 ray와의 교차 t값 (교차하지 않으면 음수)
    virtual float intersect(const Ray& ray) const = 0;
};

// Sphere: 구체, 교차 계산은 기본 2차 방정식 풀이 사용
class Sphere : public Surface {
public:
    vec3 center;
    float radius;
    Sphere(const vec3& c, float r) : center(c), radius(r) { }
    virtual float intersect(const Ray& ray) const override {
        vec3 oc = ray.origin - center;
        float b = 2.0f * dot(ray.direction, oc);
        float c_val = dot(oc, oc) - radius * radius;
        float disc = b * b - 4.0f * c_val;
        if (disc < 0.0f) return -1.0f;
        float sqrtDisc = sqrtf(disc);
        float t1 = (-b - sqrtDisc) / 2.0f;
        float t2 = (-b + sqrtDisc) / 2.0f;
        if (t1 > 0.001f) return t1;
        if (t2 > 0.001f) return t2;
        return -1.0f;
    }
};

// Plane: y = constant 평면
class Plane : public Surface {
public:
    float y;
    Plane(float yVal) : y(yVal) { }
    virtual float intersect(const Ray& ray) const override {
        if (fabs(ray.direction.y) < 1e-6f) return -1.0f;
        float t = (y - ray.origin.y) / ray.direction.y;
        return (t > 0.001f) ? t : -1.0f;
    }
};

// Camera: 눈 위치와 뷰잉 영역을 이용해 픽셀에 대응하는 ray 생성
class Camera {
public:
    vec3 eye;
    float l, r, b, t, d;
    Camera(const vec3& e, float l_, float r_, float b_, float t_, float d_)
        : eye(e), l(l_), r(r_), b(b_), t(t_), d(d_) { }
    Ray generateRay(int i, int j, int nx, int ny) const {
        float u = l + (r - l) * ((i + 0.5f) / float(nx));
        float v = b + (t - b) * ((j + 0.5f) / float(ny));
        vec3 imagePoint(u, v, -d);
        return Ray(eye, imagePoint - eye);
    }
};

// Scene: 장면 내 객체들을 보관하고, 주어진 ray와의 교차 중 가장 가까운 t값을 찾음
class Scene {
public:
    std::vector<Surface*> objects;
    ~Scene() {
        for (auto obj : objects)
            delete obj;
    }
    float findNearest(const Ray& ray) const {
        float t_nearest = -1.0f;
        for (const auto obj : objects) {
            float t = obj->intersect(ray);
            if (t > 0.0f && (t_nearest < 0.0f || t < t_nearest))
                t_nearest = t;
        }
        return t_nearest;
    }
};

// --------------------------
// 전역 변수 및 렌더링 함수
// --------------------------
int Width = 512;
int Height = 512;
std::vector<float> OutputImage;
Camera* camera = nullptr;
Scene* scene = nullptr;

// render(): 각 픽셀에 대해 ray 생성 후 교차 여부에 따라 흑백 출력
void render() {
    const int nx = 512, ny = 512;
    OutputImage.clear();
    OutputImage.resize(nx * ny * 3);

    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i) {
            Ray ray = camera->generateRay(i, j, nx, ny);
            float t = scene->findNearest(ray);
            int idx = (j * nx + i) * 3;
            if (t > 0.0f) { // 객체와 교차하면 흰색
                OutputImage[idx] = 1.0f;
                OutputImage[idx + 1] = 1.0f;
                OutputImage[idx + 2] = 1.0f;
            }
            else {         // 교차하지 않으면 검은색
                OutputImage[idx] = 0.0f;
                OutputImage[idx + 1] = 0.0f;
                OutputImage[idx + 2] = 0.0f;
            }
        }
    }
}

// --------------------------
// GLFW 콜백 및 메인 함수
// --------------------------
void resize_callback(GLFWwindow*, int nw, int nh) {
    Width = nw;
    Height = nh;
    glViewport(0, 0, nw, nh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, Width, 0.0, Height, 1.0, -1.0);
    OutputImage.reserve(Width * Height * 3);
    render();
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(Width, Height, "Simple Ray Tracer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    // 카메라: eye = (0, 0, 0), 뷰잉 영역: l = -0.1, r = 0.1, b = -0.1, t = 0.1, d = 0.1
    camera = new Camera(vec3(0.0f, 0.0f, 0.0f), -0.1f, 0.1f, -0.1f, 0.1f, 0.1f);
    scene = new Scene();

    // 장면 구성:
    // 평면 P: y = -2
    scene->objects.push_back(new Plane(-2.0f));
    // Sphere S1: center (-4, 0, -7), radius 1
    scene->objects.push_back(new Sphere(vec3(-4.0f, 0.0f, -7.0f), 1.0f));
    // Sphere S2: center (0, 0, -7), radius 2
    scene->objects.push_back(new Sphere(vec3(0.0f, 0.0f, -7.0f), 2.0f));
    // Sphere S3: center (4, 0, -7), radius 1
    scene->objects.push_back(new Sphere(vec3(4.0f, 0.0f, -7.0f), 1.0f));

    resize_callback(NULL, Width, Height);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    delete camera;
    delete scene;
    glfwTerminate();
    return 0;
}