#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <learnopengl/filesystem.h>
#include "../shader_m.h"
#include "../camera.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 1.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

struct Point3D {
    double x, y, z;

    Point3D(double x, double y, double z) : x(x), y(y), z(z) {}

    void ver(){
        std::cout<<x<<" "<<y<<" "<<z<<std::endl;
    }

    Point3D operator*(double scale) const {
        return Point3D(x * scale, y * scale, z * scale);
    }
};

// Estructura para representar un triángulo formado por 3 puntos en el espacio tridimensional
struct Triangle3D {
    Point3D p1, p2, p3;

    Triangle3D(const Point3D& p1, const Point3D& p2, const Point3D& p3) : p1(p1), p2(p2), p3(p3) {}

    std::vector<Point3D> get_points(){
        std::vector<Point3D> points;
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);
        return points;
    }
};

// Función para determinar si un punto está dentro de un triángulo en el espacio tridimensional
bool pointInTriangle3D(Point3D p, Triangle3D t) {
    // Calcula las áreas de los subtriángulos formados por el punto y los vértices del triángulo
    double mainArea = 0.5 * (
            -t.p2.y * t.p3.z + t.p1.y * (-t.p2.z + t.p3.z) + t.p1.z * (t.p2.y - t.p3.y) +
            t.p2.x * t.p3.z - t.p1.x * (t.p2.z - t.p3.z) - t.p1.z * (t.p2.x - t.p3.x) +
            t.p2.x * t.p3.y - t.p1.x * (t.p2.y - t.p3.y) + t.p1.y * (t.p2.x - t.p3.x)
    );

    double area1 = 0.5 * (
            -p.y * t.p3.z + t.p1.y * (-p.z - t.p3.z) + t.p1.z * (p.y - t.p3.y) +
            p.x * t.p3.z - t.p1.x * (p.z - t.p3.z) - t.p1.z * (p.x - t.p3.x) +
            p.x * t.p3.y - t.p1.x * (p.y - t.p3.y) + t.p1.y * (p.x - t.p3.x)
    );

    double area2 = 0.5 * (
            -t.p2.y * p.z + p.y * (-t.p2.z + t.p3.z) + t.p1.z * (t.p2.y - p.y) +
            t.p2.x * p.z - t.p1.x * (t.p2.z - p.z) - t.p1.z * (t.p2.x - p.x) +
            t.p2.x * t.p3.y - t.p1.x * (t.p2.y - t.p3.y) + p.x * (t.p2.x - t.p3.x)
    );

    double area3 = 0.5 * (
            -t.p2.y * t.p3.z + t.p1.y * (-t.p2.z - t.p3.z) + p.x * (t.p2.y - t.p3.y) +
            t.p2.x * t.p3.z - t.p1.x * (t.p2.z + t.p3.z) - t.p1.z * (t.p2.x + t.p3.x) +
            t.p2.x * p.y - p.x * (t.p2.y - t.p3.y) + t.p1.y * (t.p2.x + t.p3.x)
    );

    // Si la suma de las áreas de los subtriángulos es igual al área del triángulo principal,
    // el punto está dentro del triángulo
    return area1 + area2 + area3 == mainArea;
}

// Función para generar los triángulos de la superficie tridimensional
std::vector<Triangle3D> generateTriangles3D(std::vector<Point3D>& points) {
    std::vector<Triangle3D> triangles;

    // Triangulación de Delaunay utilizando el algoritmo incremental de Bowyer-Watson en 3D
    // Agrega el primer triángulo a la lista de triángulos
    triangles.push_back(Triangle3D(points[0], points[1], points[2]));

    for (size_t i = 1; i < points.size(); i++) {
        Point3D currentPoint = points[i];
        std::vector<Triangle3D> validTriangles;

        // Itera sobre los triángulos existentes
        for (auto triangle : triangles) {
            std::cout<<i<<std::endl;

            // Verifica si el punto está dentro del triángulo
        //    if (pointInTriangle3D(currentPoint, triangle)) {
                // Los vértices del triángulo original se convierten en los vértices de 4 nuevos triángulos
                validTriangles.push_back(Triangle3D(triangle.p1, triangle.p2, currentPoint));
                validTriangles.push_back(Triangle3D(triangle.p2, triangle.p3, currentPoint));
                validTriangles.push_back(Triangle3D(triangle.p3, triangle.p1, currentPoint));
                validTriangles.push_back(Triangle3D(currentPoint, triangle.p1, triangle.p2));
          //  } else {
                // Si el punto no está dentro del triángulo, se agrega el triángulo a la lista de triángulos válidos
//                validTriangles.push_back(triangle);
            //}
        }
        triangles = std::move(validTriangles);

    }

    return triangles;
}




int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    std::string pre = "../2.1.basic_lighting_diffuse/";
    Shader lightingShader((pre+"2.1.basic_lighting.vs").c_str(), (pre+"2.1.basic_lighting.fs").c_str());
    Shader lightCubeShader((pre+"2.1.light_cube.vs").c_str(), (pre+"2.1.light_cube.fs").c_str());

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

/*
float vertices2[] = {
    //Cara opuesta luz
-0.5f, -0.5f, -0.5f,  0.0f,  -1.0f, 0.5f,
 0.5f, -0.5f, -0.5f,  0.0f,  -1.0f, 0.5f,
 0.0f,  0.5f, -0.0f,  0.0f,  -1.0f, 0.5f,

 //Cara luz
-0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  1.0f,
0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  1.0f,
0.0f,  0.5f,  0.0f,  0.0f,  0.5f,  1.0f,

//Cara otra
-0.5f, -0.5f, -0.5f, -1.0f,  1.0f,  0.0f,
-0.5f, -0.5f,  0.5f, -1.0f,  1.0f,  0.0f,
0.0f,  0.5f,  0.0f, -1.0f,  1.0f,  0.0f,


//Cara opuesta otra
0.5f, -0.5f, -0.5f,  1.0f,  -1.0f,  0.0f,
0.5f, -0.5f,  0.5f,  1.0f,  -1.0f,  0.0f,
0.0f,  0.5f,  0.0f,  1.0f,  -1.0f,  0.0f,


 //Base piramide
-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

};

*/

/*
    int total_points = 18;
    int point = 0;
    float vertices[total_points];

    while(point<total_points){

        std::cout<<point<<std::endl;
        vertices[point] = std::rand() % 5 + 1;
        point++;
        vertices[point] = std::rand() % 6 + 1;
        point++;
        vertices[point] = std::rand() % 7 + 1;
        point++;
        vertices[point] = 0.0f;
        point++;
        vertices[point] = -0.5f;
        point++;
        vertices[point] = -1.0f;
        point++;

    }

const double radius = 1.0; // Radio de la esfera
    const int numPoints = 10;
    std::vector<Point3D> points;
    for(float x=-10.0; x < 10.0; x+=0.1){
        for(float y=-10.0; y < 10.0; y+=0.1) {
            float z = 5 - x * x - y * y;
            points.emplace_back(Point3D(x, y, z));
        }
    }
*/
    std::vector<Point3D> points;

    /*
            points.push_back(Point3D(0, 0, 0));
            points.push_back(Point3D(1, 0, 0));
            points.push_back(Point3D(0, 1, 0));
            points.push_back(Point3D(1, 1, 0));
            points.push_back(Point3D(0.5, 0.5, 1));
*/
    points.push_back(Point3D(20,25,14));
    points.push_back(Point3D(25, 20, 14));
    points.push_back(Point3D(25, 14, 20));
    points.push_back(Point3D(31.6, 10, 10));
//    points.push_back(Point3D);
//    points.push_back(Point3D);

    std::vector<Point3D> dodecahedronPoints;

    const double phi = (1 + sqrt(5.0)) / 2.0;
    const double invPhi = 1 / phi;
    std::cout<<phi<<std::endl;
    // Generación de los puntos de la mitad del dodecaedro
    dodecahedronPoints.push_back(Point3D(0, invPhi, phi));
    dodecahedronPoints.push_back(Point3D(0, -invPhi, phi));
    dodecahedronPoints.push_back(Point3D(0, invPhi, -phi));
    dodecahedronPoints.push_back(Point3D(0, -invPhi, -phi));
    dodecahedronPoints.push_back(Point3D(phi, 0, invPhi));
    dodecahedronPoints.push_back(Point3D(phi, 0, -invPhi));
    dodecahedronPoints.push_back(Point3D(-phi, 0, invPhi));
    dodecahedronPoints.push_back(Point3D(-phi, 0, -invPhi));
    dodecahedronPoints.push_back(Point3D(invPhi, phi, 0));
    /*

    dodecahedronPoints.push_back(Point3D(-invPhi, phi, 0));

    dodecahedronPoints.push_back(Point3D(invPhi, -phi, 0));
    dodecahedronPoints.push_back(Point3D(-invPhi, -phi, 0));
    */
    /*
    for (int i = 0; i < numPoints; i++) {
        double theta = i * (2.0 * M_PI / numPoints); // Ángulo polar
        for (int j = 0; j < numPoints; j++) {
            double phi = j * (2.0 * M_PI / numPoints); // Ángulo azimutal

            Point3D cartesianPoint = sphericalToCartesian(radius, theta, phi);
            cartesianPoint.ver();
            points.push_back(cartesianPoint);
        }
    }
     */
    // Agrega aquí los puntos adicionales de tu conjunto
    std::cout<<"hola"<<std::endl;


    std::vector<Triangle3D> triangles = generateTriangles3D(points);


    float vertices[triangles.size()*18];
    int point = 0;
    std::cout<<"size "<<triangles.size()<<std::endl;
    for (auto triangle : triangles) {
        for(auto p : triangle.get_points()){
            p.ver();
            vertices[point] = p.x;
            point++;
            vertices[point] = p.y;
            point++;
            vertices[point] = p.z;
            point++;
            vertices[point] = 0.0f;
            point++;
            vertices[point] = -0.5f;
            point++;
            vertices[point] = -1.0f;
            point++;
        }

    }



        // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);

    glBindVertexArray(lightCubeVAO);

    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("lightPos", lightPos);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // render the cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
