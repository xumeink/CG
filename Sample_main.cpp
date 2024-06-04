#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GL/glut.h>

#include <GL/glm/glm.hpp>
#include <GL/glm/gtx/transform.hpp> // rotate(), scale(), translate()
#include <GL/glm/gtc/quaternion.hpp>
#include <GL/glm/gtc/type_ptr.hpp>

using namespace std;

GLuint VertexArrayID;
GLuint programID;
GLint modelLoc;
GLint viewLoc;
GLint projLoc;
float sx = 0;
float sy = 0;
float sz = 0;
float dx = 0;
float dy = 0;
float dz = 0;
float translationStep = 0.1f;
float rotationAngle = 0.0f;
float rotationAngle2 = 0.0f;
float rotationStep = 5.0f;
bool usePerspectiveProj = true; // false: ortho
bool useGouraudShading = true; // false: phong shading

glm::vec3 cameraPosition(5.0f, 3.0f, 5.0f); // 카메라 위치
glm::vec3 targetPosition(2.0f, 2.0f, 2.0f); // 타겟(큐브) 위치
glm::vec3 upVector(0.0f, 1.0f, 0.0f); // 업 벡터
glm::mat4 view = glm::lookAt(cameraPosition, targetPosition, upVector);

glm::vec3 pointLightPos = glm::vec3(2.0f, 2.0f, 2.0f);
glm::vec3 pointLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 directLightDir = glm::vec3(-2.0f, -2.0f, -2.0f);
glm::vec3 directLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

GLint pointLightPosLoc;
GLint pointLightColorLoc;
GLint directLightDirLoc;
GLint directLightColorLoc;

GLfloat cubeVertices[] = {
    0.1f,  0.1f,  0.1f, // v0
   -0.1f,  0.1f,  0.1f, // v1
   -0.1f, -0.1f,  0.1f, // v2
    0.1f, -0.1f,  0.1f, // v3
    0.1f, -0.1f, -0.1f, // v4
    0.1f,  0.1f, -0.1f, // v5
   -0.1f,  0.1f, -0.1f, // v6
   -0.1f, -0.1f, -0.1f  // v7
};

GLfloat cubeColors[] = {
    1.0f, 0.0f, 0.0f, 1.0f, // Red
    0.0f, 1.0f, 0.0f, 1.0f, // Green
    0.0f, 0.0f, 1.0f, 1.0f, // Blue
    1.0f, 1.0f, 0.0f, 1.0f, // Yellow
    1.0f, 0.0f, 1.0f, 1.0f, // Magenta
    0.0f, 1.0f, 1.0f, 1.0f,  // Cyan
};

GLint cubeIndices[] = {
    //front
    0,1,2,
    0,2,3,
    //right
    0,3,4,
    0,4,5,
    //up
    0,1,6,
    0,6,5,
    //left
    1,2,7,
    1,7,6,
    //down
    3,7,4,
    3,2,7,
    //back
    5,7,6,
    5,4,7,
};

GLfloat cubeNormals[] = {
    0.0f, 0.0f, 1.0f, // front
    1.0f, 0.0f, 0.0f, // right
    0.0f, 1.0f, 0.0f, // up
   -1.0f, 0.0f, 0.0f, // left
    0.0f, -1.0f, 0.0f, // down
    0.0f, 0.0f, -1.0f, // back
};

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    //create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    //Read the vertex shader code from the file
    string VertexShaderCode;
    ifstream VertexShaderStream(vertex_file_path, ios::in);
    if (VertexShaderStream.is_open())
    {
        string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    //Compile Vertex Shader
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);
    printf("Compiling shader : %s\n", vertex_file_path);

    //Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        vector<char> VertexShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    }


    //Read the fragment shader code from the file
    string FragmentShaderCode;
    ifstream FragmentShaderStream(fragment_file_path, ios::in);
    if (FragmentShaderStream.is_open())
    {
        string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    //Compile Fragment Shader
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);
    printf("Compiling shader : %s\n", fragment_file_path);

    //Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        vector<char> FragmentShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
    }


    //Link the program
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    fprintf(stdout, "Linking program\n");

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void setProjectionMatrix() {
    glm::mat4 proj;
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    if (usePerspectiveProj) {
        // perspective projection
        proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }
    else {
        // orthogonal projection
        proj = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, -15.0f, 15.0f);
    }

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
}

void switchShader() {
    useGouraudShading = !useGouraudShading;
    std::cout << "Switched to " << (useGouraudShading ? "Gouraud" : "Phong") << " shading" << std::endl;

    glDeleteProgram(programID);
    if (useGouraudShading) {
        programID = LoadShaders("GVertexShader.txt", "GFragmentShader.txt");
    }
    else {
        programID = LoadShaders("PVertexShader.txt", "PFragmentShader.txt");
    }
    glUseProgram(programID);

    modelLoc = glGetUniformLocation(programID, "model");
    viewLoc = glGetUniformLocation(programID, "view");
    projLoc = glGetUniformLocation(programID, "proj");
    pointLightPosLoc = glGetUniformLocation(programID, "pointLightPos");
    pointLightColorLoc = glGetUniformLocation(programID, "pointLightColor");
    directLightDirLoc = glGetUniformLocation(programID, "directLightDir");
    directLightColorLoc = glGetUniformLocation(programID, "directLightColor");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    setProjectionMatrix();
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));
    glUniform3fv(directLightDirLoc, 1, glm::value_ptr(directLightDir));
    glUniform3fv(directLightColorLoc, 1, glm::value_ptr(directLightColor));

    glutPostRedisplay();
}

void renderScene(void)
{
    //Clear all pixels
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Let's draw something here

    glBindVertexArray(VertexArrayID);

    //define the size of point and draw a point.
    int numCubes = 10;
    for (int i = 0; i < numCubes; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 translateVec(0.0f + sx, i * 0.3f + sy, 0.0f + sz);
        model = glm::translate(model, translateVec);

        glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glm::vec3 pointLightPos = glm::vec3(2.0f+dx, 2.0f+dy, 2.0f+dz);
        glm::vec3 directLightDir = glm::vec3(-2.0f, -2.0f+ rotationAngle2, -2.0f);

        glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
        glUniform3fv(directLightDirLoc, 1, glm::value_ptr(directLightDir));

        glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(GLint), GL_UNSIGNED_INT, 0);
    }

    //Double buffer
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1': // gouraud -> phong
        useGouraudShading = true;
        switchShader();
        break;
    case '2': // phong -> gouraud
        useGouraudShading = false;
        switchShader();
        break;
    case 'q':
        sx += translationStep;
        break;
    case 'w':
        sx -= translationStep;
        break;
    case 'a':
        sy += translationStep;
        break;
    case 's':
        sy -= translationStep;
        break;
    case 'z':
        sz += translationStep;
        break;
    case 'x':
        sz -= translationStep;
        break;
    case 'o':
        dx += translationStep;
        break;
    case 'p':
        dx -= translationStep;
        break;
    case 'k':
        dy += translationStep;
        break;
    case 'l':
        dy -= translationStep;
        break;
    case 'n':
        dz += translationStep;
        break;
    case 'm':
        dz -= translationStep;
        break;
    }

    pointLightPos = glm::vec3(2.0f + dx, 2.0f + dy, 2.0f + dz);
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));

    glutPostRedisplay();
}

void specialKey(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        rotationAngle -= rotationStep;
        break;
    case GLUT_KEY_RIGHT:
        rotationAngle += rotationStep;
        break;
    case GLUT_KEY_UP:
        rotationAngle2 += rotationStep;
        break;
    case GLUT_KEY_DOWN:
        rotationAngle2 -= rotationStep;
        break;
    }

    directLightDir = glm::vec3(-2.0f, -2.0f + rotationAngle2, -2.0f);
    glUniform3fv(directLightDirLoc, 1, glm::value_ptr(directLightDir));

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        usePerspectiveProj = !usePerspectiveProj;
        setProjectionMatrix();
        glutPostRedisplay();
    }
}

void init()
{
    //initilize the glew and check the errors.
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
    }

    //select the background color
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS);
    glDepthRange(0.0f, 1.0f);
}

int main(int argc, char** argv)
{
    //init GLUT and create Window
    //initialize the GLUT
    glutInit(&argc, argv);
    //GLUT_DOUBLE enables double buffering (drawing to a background buffer while the other buffer is displayed)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    //These two functions are used to define the position and size of the window. 
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(480, 480);
    //This is used to define the name of the window.
    glutCreateWindow("Simple OpenGL Window");

    //call initization function
    init();
    //0.
    
    programID = LoadShaders("GVertexShader.txt", "GFragmentShader.txt");
    glUseProgram(programID);

    /**************************************************/
    // 모델, 뷰, 프로젝션 행렬을 셰이더에 전달
    modelLoc = glGetUniformLocation(programID, "model");
    viewLoc = glGetUniformLocation(programID, "view");
    projLoc = glGetUniformLocation(programID, "proj");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // 프로젝션 행렬 설정
    setProjectionMatrix();


    // 점 광원의 위치, 색상을 셰이더에 전달
    pointLightPosLoc = glGetUniformLocation(programID, "pointLightPos");
    pointLightColorLoc = glGetUniformLocation(programID, "pointLightColor");
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    // 방향 광원의 위치, 색상을 셰이더에 전달
    directLightDirLoc = glGetUniformLocation(programID, "directLightDir");
    directLightColorLoc = glGetUniformLocation(programID, "directLightColor");
    glUniform3fv(directLightDirLoc, 1, glm::value_ptr(directLightDir));
    glUniform3fv(directLightColorLoc, 1, glm::value_ptr(directLightColor));

    /**************************************************/
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    GLuint VBOs[3];
    glGenBuffers(3, VBOs);
    GLuint EBO[1];
    glGenBuffers(1, EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    GLuint posAttribLoc = glGetAttribLocation(programID, "inPos");
    glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
    glEnableVertexAttribArray(posAttribLoc);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
    GLuint colorAttribLoc = glGetAttribLocation(programID, "inColor");
    glVertexAttribPointer(colorAttribLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
    glEnableVertexAttribArray(colorAttribLoc);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
    GLuint normalAttribLoc = glGetAttribLocation(programID, "inNormal");
    glVertexAttribPointer(normalAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
    glEnableVertexAttribArray(normalAttribLoc);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    
    glutDisplayFunc(renderScene);
    glutKeyboardFunc(keyboard); // translate cube, point light and swith shader program
    glutSpecialFunc(specialKey); // rotate cube and directional light
    glutMouseFunc(mouse); // switch projection mode

    //enter GLUT event processing cycle
    glutMainLoop();

    glDeleteVertexArrays(1, &VertexArrayID);

    return 1;
}