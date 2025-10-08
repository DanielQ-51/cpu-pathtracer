/*
Code by Daniel Qin, qindan@seas.upenn.edu


Contains the main handles the overall render loop, along with the initialization of the scene. Most of the actual
computation is done in functions defined in other classes. This file abstracts away a lot of the details by using
BSDF, Sampler, and Integrator objects with their own functions.

The overall object structure is inspired by PBRT, which uses inheritance to extend generic integrator and BSDF and
sampler objects. Currently this code only uses inheritance for the different BSDF models, but it is organized to 
easily provide support for different types of Integrators and Samplers with a little bit more work.

Currently, it is very easily to change out what materials are used for what meshes by using the BSDF classes.

*/


#include "image.h"
#include "bmp.h"
#include "object.h"
#include "lightTransport.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <limits>
#include <fstream>
#include <sstream>

#include <atomic>
#include <iomanip>

#include <omp.h>

using namespace std;


void readObj(string filename, vector<Vertex>& vertices, vector<Triangle>& mesh, vector<Triangle>& lights, Color c, Color e, BSDF& material);

int main () {
    auto start = std::chrono::high_resolution_clock::now();

    // Image setup
    int imageWidth = 2000;
    int imageHeight = 2000;

    Image testImage(imageWidth,imageHeight);

    // Camera setup
    double viewPortWidth = 1;
    double viewPortHeight = 1;
    Point cameraOrigin(0,0,1.0);

    // Number of samples per pixel
    int sampleCount = 30;

    // Initialization of data structures to store scene
    // (Triangle objects contain pointers to vertices)
    vector<Triangle> objects;
    vector<Triangle> lights;

    // Stores all vertices continguously in memory for performance
    vector<Vertex> vertices;

    // Pre allocates space, since the file reading does not work well when it has to dynamically allocate memory
    vertices.reserve(20000);

    //Material (BSDF) initialization

    //phongBSDF DiffuseReflector = phongBSDF();
    //DiffuseReflector.phongExponent = 1;
    mirrorBSDF ShinyReflector = mirrorBSDF();

    simpleDiffuseBSDF DiffuseReflector = simpleDiffuseBSDF();

    MISIntegrator integrator = MISIntegrator();
    integrator.maxDepth = 6;


    //Mesh creation
    readObj("largebox.obj", vertices, objects, lights, Color(1.0,1.0,1.0), Color(0,0,0), DiffuseReflector);
    readObj("leftwall.obj", vertices, objects, lights, Color(1.0,0.0,0.0), Color(0,0,0), DiffuseReflector);
    readObj("rightwall.obj", vertices, objects, lights , Color(0.0,1.0,0.0), Color(0,0,0), DiffuseReflector);

    readObj("box1.obj", vertices, objects, lights, Color(1.0,1.0,1.0), Color(0,0,0), DiffuseReflector);
    readObj("widebox.obj", vertices, objects, lights, Color(1.0,1.0,0.6), Color(0,0,0), ShinyReflector);
    
    //
    //readObj("light.obj", vertices, objects, lights, Color(1.0,1.0,0.6), 1*Color(10,10,6), DiffuseReflector);
    readObj("smalllight.obj", vertices, objects, lights, Color(1.0,1.0,0.6), 30*Color(10,10,6), DiffuseReflector);


    // to handle progress bar 
    std::atomic<int> pixelsDone(0);
    
    // Multithreading setup
    int maxThreads = omp_get_max_threads();      
    int useThreads = int(maxThreads * 0.9); // Set the float value to the % of CPU you want to use
    if (useThreads < 1) useThreads = 1;
    omp_set_num_threads(useThreads);

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < imageWidth; i++)
    {
        for(int j = 0; j < imageHeight; j++)
        {
            std::random_device rd;
            SimpleSampler sampler(rd() + j * imageWidth + i);
            Color L = Color(0.0, 0.0, 0.0);
            for (int k = 0; k < sampleCount; k++)
            {
                auto [du, dv] = sampler.get2D();
                Point a = Point((i + 1.0*du - 0.5 - imageWidth/2.0) * (viewPortWidth / imageWidth), 
                    (j + 1.0*dv - 0.5 - imageHeight/2.0) * (viewPortHeight / imageHeight), 0);
                Ray r = Ray(a - cameraOrigin, cameraOrigin);
                Color l = integrator.Li(objects, lights, r, sampler);
                L += l;
            }
            L /= (double)sampleCount;
            testImage.setColor(i, j, L);

            // Progressively save the image as it renders, and update progress bar
            int done = ++pixelsDone;
            if (done % 100000 == 0 || done == imageHeight*imageWidth) {
                float progress = (done / float(imageHeight*imageWidth)) * 100.0f;
                #pragma omp critical
                {
                    std::cout << "\rProgress: " << std::fixed << std::setprecision(2)
                            << progress << "% " << std::flush;
                }
            }

            if (done % 1000000 == 0 || done == imageHeight*imageWidth) {
                testImage.saveImageBMP("render.bmp");
            }
        }
    }
    cout << endl;
    
    // output timekeeping stuff
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds" << std::endl;

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed time: " << elapsed_ms.count() << " milliseconds" << std::endl;

    // Save image
    testImage.saveImageBMP("render.bmp");
}

// reads in obj files and assigns the data to triangles
void readObj(string filename, vector<Vertex>& vertices, vector<Triangle>& mesh, vector<Triangle>& lights, Color c, Color e, BSDF& material)
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open OBJ file\n";
        return;
    }

    vector<Point> points;
    vector<Vec3> normals;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // skip comments

        std::istringstream iss(line);
        std::string prefix;
        
        iss >> prefix;

        if (prefix == "v") {
            double x, y, z;
            iss >> x >> y >> z;
            Point p(x,y,z);
            points.push_back(p);
        }
        else if (prefix == "vt") {}
        else if (prefix == "vn") {
            double x, y, z;
            iss >> x >> y >> z;
            Vec3 n(x,y,z);
            normals.push_back(n);
        }
        else if (prefix == "f") {
            vector<string> items;

            string vertinfo;
            vector<int> vertexIndices;
            vector<int> normalIndices;
            while (iss >> vertinfo) 
            {
                istringstream vss(vertinfo);
                string idx;

                if (getline(vss, idx, '/'))
                {
                    if (!idx.empty())
                        vertexIndices.push_back(stoi(idx) - 1);
                }
                if (getline(vss, idx, '/'))
                {

                }
                if (getline(vss, idx, '/'))
                {
                    if (!idx.empty())
                        normalIndices.push_back(stoi(idx) - 1);
                }
            }
            int n = vertexIndices.size();
            if (n == 3)
            {
                int startIndex = vertices.size(); // current end of vector

                // create vertices and store them in the vector
                vertices.push_back(Vertex(points[vertexIndices[0]], c, normals[normalIndices[0]]));
                vertices.push_back(Vertex(points[vertexIndices[1]], c, normals[normalIndices[1]]));
                vertices.push_back(Vertex(points[vertexIndices[2]], c, normals[normalIndices[2]]));

                // pointers to the vertices stored in the vector
                Vertex* v1 = &vertices[startIndex];
                Vertex* v2 = &vertices[startIndex + 1];
                Vertex* v3 = &vertices[startIndex + 2];

                // create triangle using the pointers
                mesh.push_back(Triangle(v1, v2, v3, e, &material));
                if (e.lengthSquared() > 0)
                    lights.push_back(Triangle(v1, v2, v3, e, &material));
            }
            else if (n == 4)
            {
                int startIndex = vertices.size();

                vertices.push_back(Vertex(points[vertexIndices[0]], c, normals[normalIndices[0]]));
                vertices.push_back(Vertex(points[vertexIndices[1]], c, normals[normalIndices[1]]));
                vertices.push_back(Vertex(points[vertexIndices[2]], c, normals[normalIndices[2]]));
                vertices.push_back(Vertex(points[vertexIndices[3]], c, normals[normalIndices[3]]));

                Vertex* v1 = &vertices[startIndex];
                Vertex* v2 = &vertices[startIndex + 1];
                Vertex* v3 = &vertices[startIndex + 2];
                Vertex* v4 = &vertices[startIndex + 3];

                // split quad into two triangles
                mesh.push_back(Triangle(v1, v2, v3, e, &material));
                mesh.push_back(Triangle(v1, v3, v4, e, &material));

                if (e.lengthSquared() > 0)
                {
                    lights.push_back(Triangle(v1, v2, v3, e, &material));
                    lights.push_back(Triangle(v1, v3, v4, e, &material));
                }

            }
            else if (n > 4) 
            {
                int startIndex = vertices.size();
                std::vector<Vertex*> polyVertices;
                polyVertices.reserve(n);

                for (int i = 0; i < n; ++i)
                {
                    // Create the vertex and add it to the main list
                    vertices.push_back(Vertex(
                        points[vertexIndices[i]], 
                        c, 
                        normals[normalIndices[i]]
                    ));
                    polyVertices.push_back(&vertices[startIndex + i]);
                }

                for (int i = 1; i < n - 1; ++i)
                {
                    Vertex* v1 = polyVertices[0];
                    Vertex* v2 = polyVertices[i];
                    Vertex* v3 = polyVertices[i + 1];
                    
                    mesh.push_back(Triangle(v1, v2, v3, e, &material));

                    if (e.lengthSquared() > 0)
                    {
                        lights.push_back(Triangle(v1, v2, v3, e, &material));
                    }
                }
            }
        }
    }

    file.close();
}


// Old function that would manually create a scene; used before the readObj() function was implemented
/*
void constructScene(vector<Triangle>& obj, vector<Triangle>& l)
{
    Point p1 = Point(0, 0, -2);
    Point p2 = Point(1, 0, -1);
    Point p3 = Point(0, 1, -2);

    Vec3 normVector = cross(p1-p2, p2-p3);
    cout << normVector << endl;

    Vertex vert1 = Vertex(p1, Color(0.1, 0.1, 0.3), normVector); //r
    Vertex vert2 = Vertex(p2, Color (0.1, 0.3,0.1), normVector); //g
    Vertex vert3 = Vertex(p3, Color(0.3, 0.1, 0.1), normVector); // b

    Triangle maintri = Triangle(vert1,vert2,vert3, Color(0.0, 0.0, 0.0));

    Point p10 = Point(0, 0, -2);
    Point p20 = Point(-0.5, 0, -1);
    Point p30 = Point(0, 1, -2);

    normVector = -cross(p10-p20, p20-p30);
    cout << normVector << endl;

    vert1 = Vertex(p10, Color(0.1, 0.1, 0.3), normVector); //r
    vert2 = Vertex(p20, Color (0.1, 0.3,0.1), normVector); //g
    vert3 = Vertex(p30, Color(0.3, 0.1, 0.1), normVector); // b

    Triangle sidetri = Triangle(vert1,vert2,vert3, Color(0.0, 0.0, 0.0));
    
    Point p14 = Point(1, 0, -1);
    Point p24 = Point(0, 0, -2);
    Point p34 = Point(-0.5, 0, -1);

    normVector = cross(p14-p24, p24-p34);
    cout << normVector << endl;

    vert1 = Vertex(p14, Color(0.1, 0.1, 0.3), normVector); //r
    vert2 = Vertex(p24, Color (0.1, 0.3,0.1), normVector); //g
    vert3 = Vertex(p34, Color(0.3, 0.1, 0.1), normVector); // b

    Triangle bottomtri = Triangle(vert1,vert2,vert3, Color(0.0, 0.0, 0.0));

    Point p11 = Point(-100, -100, 1.1);
    Point p21 = Point(100, 300, 0.8);
    Point p31 = Point(100, -100, 1.1);

    normVector = cross(p11-p21, p21-p31);
    cout << normVector << endl;

    vert1 = Vertex(p11, Color(1.0, 1.0, 1.0), normVector);
    vert2 = Vertex(p21, Color (1.0, 1.0, 1.0), normVector);
    vert3 = Vertex(p31, Color(1.0, 1.0, 1.0), normVector);

    Triangle light = Triangle(vert1,vert2,vert3, Color(0, 10.0, 0));

    Point p12 = Point(0.4, 0.5, -1.5);
    Point p22 = Point(0, 0.5, -1.6);
    Point p32 = Point(0, 0.5, -1.6);

    normVector = -cross(p12-p22, p22-p32);
    cout << normVector << endl;

    vert1 = Vertex(p12, Color(1.0, 1.0, 1.0), normVector);
    vert2 = Vertex(p22, Color (1.0, 1.0, 1.0), normVector);
    vert3 = Vertex(p32, Color(1.0, 1.0, 1.0), normVector);

    Triangle othertri = Triangle(vert1,vert2,vert3, Color(400.0, 0, 0));

    Point p13 = Point(0.3, 0, -1.20);
    Point p23 = Point(0.4, 0.1, -1.10);
    Point p33 = Point(0.3, 0.15, -1.20);

    normVector = cross(p13-p23, p23-p33);
    cout << normVector << endl;

    vert1 = Vertex(p13, Color(0.1, 0.1, 0.1), normVector);
    vert2 = Vertex(p23, Color (0.1, 0.1, 0.1), normVector);
    vert3 = Vertex(p33, Color(0.1, 0.1, 0.1), normVector);

    Triangle tri3 = Triangle(vert1,vert2,vert3, Color(0, 0, 0));

    obj = {light, maintri, othertri, sidetri, bottomtri};
    l = {light};
}*/