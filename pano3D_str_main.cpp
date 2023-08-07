// Modified from "Video Texture" code as C++ language
// add the panorama texture to 3D model (by jekim)
// url :: https://gist.github.com/Ashwinning/baeb0835624fedc2e5b809d42417b70e

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp> 
#include <iostream>
#include <GL/glut.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include "./pano_str_h.h"

#define KEY_ESCAPE 27 // 'ESC'

// global variable
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
const float CAMERA_DISTANCE = 10.0f;
int frame_width  = 1920;
int frame_height = 1080;
double fps;
bool mirror = true;

cv::VideoCapture cap_center(4), cap_left(16), cap_right(10);
cv::Mat image; // panorama image
cv::Mat image_c, image_l, image_r; 
GLint g_hWindow;
Panorama *panorama = new Panorama(); 

// function declaration 
double current_time_in_seconds();
GLvoid init_glut();
GLvoid display();
GLvoid reshape(GLint w, GLint h);
GLvoid key_press (unsigned char key, GLint x, GLint y);
GLvoid idle();
void mouseMotionCB(int x, int y);
void mouseCB(int button, int state, int x, int y);

// main loop
int main(int argc, char* argv[])
{
    int width = 1920;
    int height = 1080; 
    cap_center.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap_center.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap_left.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap_left.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap_right.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap_right.set(cv::CAP_PROP_FRAME_HEIGHT, height);

    int frame_height = height;
    int frame_width = width; 

    // Create GLUT Window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(frame_width*5, frame_height*3);

    g_hWindow = glutCreateWindow("Video Texture");

    // Initialize OpenGL
    init_glut();

    glutMainLoop();

    return 0;
}

// function definition
double current_time_in_seconds()
{
    timeval timer;
    gettimeofday(&timer,NULL);
    double seconds = 1e-6 * timer.tv_usec + timer.tv_sec;
    return seconds;
}

GLvoid init_glut()
{  
    glClearColor (0.0, 0.0, 0.0, 0.0);
    // Set up callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key_press);
    glutMouseFunc(mouseCB); // (8)
    glutMotionFunc(mouseMotionCB); //(9)
    glutIdleFunc(idle);
}

GLvoid display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LINE_STIPPLE);
    // These are necessary if using glTexImage2D instead of gluBuild2DMipmaps
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // use when you want to show the patten of polygon

    // Set Projection Matrix
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    // gluOrtho2D(0, frame_width, frame_height, 0); // setting the coordinate, default is set to be "glOrtho(-1,1,-1,1,-1,1)"

    // Switch to Model View Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glPushMatrix(); 
    // glRotatef(cameraAngleX, 1, 0, 0);
    // glRotatef(cameraAngleY, 0, 1, 0);
    // std::cout<<"X: "<<cameraAngleX<<std::endl;
    // std::cout<<"Y: "<<cameraAngleY<<std::endl; // use these lines when you want to change the rotation matrix using mouse input
    glRotatef(260, 1, 0, 0); 
    glRotatef(-180, 0, 1, 0);
    glRotatef(90, 0, 0, 1);
    glTranslatef(0, 0, 0.3); 

    // //// 3D Model: hemisphere+cylinder (plz, recommand the new model which is more suitable to panorama scene)
    float r = 0.8; 
    float rp1, rp2; 
    float pi = 3.141592;
    float a_i=0.5;int num_i=40;
    float a_jc=0.3; // maximum angle to draw cylinder in terms of spherical coordinate is 0.25 
    float a_js=0.13; int num_j=num_i; // angle for sphere, a_js is recommand to be mode(0.5/a_js)=0 
    float di = 2*a_i/num_i;
    float djc = (a_jc+a_js)/num_j;
    float djs = (a_jc+a_js)/num_j;
    float db = di * pi;
    float dac = djc * pi;
    float das = djs * pi;
    float dii=1/float(num_i); 
    float djjc=1/float(num_j);
    float djjs=1/float(num_j);

    for (float i = -a_i; i < a_i-di; i += di) //horizonal
    {
        for (float j = -a_jc; j <0; j += djc) //vertical, cylinder
        {
            float b = i * pi;      //0     to  2pi
            float a = j * pi;
            float ii = i+a_i; ii = ii/(2*a_i);
            float jj = j+a_jc; jj = jj/(a_jc+a_js); 
            rp1=r / cos (a);
            rp2=r / cos (a+dac); 

            //normal
            glNormal3f(
                cos(a + dac / 2) * cos(b + db / 2),
                cos(a + dac / 2) * sin(b + db / 2),
                sin(a + dac / 2));

            glBegin(GL_QUADS);
            //P1
                glTexCoord2f(ii, jj);
                glVertex3f(
                    rp1 * cos(a) * cos(b),
                    rp1 * cos(a) * sin(b),
                    rp1 * sin(a));
                // glColor3f(0,0,0);
                
            //P2
                glTexCoord2f(ii+dii, jj);
                glVertex3f(
                    rp1 * cos(a) * cos(b + db),
                    rp1 * cos(a) * sin(b + db),
                    rp1 * sin(a));
                // glColor3f(1,0,0);

            //P3
                glTexCoord2f(ii+dii, jj+djjc);
                glVertex3f(
                    rp2 * cos(a + dac) * cos(b + db),
                    rp2 * cos(a + dac) * sin(b + db),
                    rp2 * sin(a + dac));
                // glColor3f(1,1,0);

            //P4
                glTexCoord2f(ii,jj+djjc);
                glVertex3f(
                    rp2 * cos(a + dac) * cos(b),
                    rp2 * cos(a + dac) * sin(b),
                    rp2 * sin(a + dac));
                // glColor3f(1,0,1);
            glEnd();
        }

        for (float j = 0; j <a_js-djs; j += djs) //vertical, hemisphere
        {
            float b = i * pi;      //0     to  2pi
            float a = j * pi;
            float ii = i+a_i; ii = ii/(2*a_i);
            float jj = j+a_jc; jj = jj/(a_jc+a_js); 

            //normal
            glNormal3f(
                cos(a + das / 2) * cos(b + db / 2),
                cos(a + das / 2) * sin(b + db / 2),
                sin(a + das / 2));

            glBegin(GL_QUADS);
            //P1
                glTexCoord2f(ii, jj);
                glVertex3f(
                    r * cos(a) * cos(b),
                    r * cos(a) * sin(b),
                    r * sin(a));
                // glColor3f(0,0,0);
                
            //P2
                glTexCoord2f(ii+dii, jj);
                glVertex3f(
                    r * cos(a) * cos(b + db),
                    r * cos(a) * sin(b + db),
                    r * sin(a));
                // glColor3f(1,0,0);
            //P3
                glTexCoord2f(ii+dii, jj+djjs);
                glVertex3f(
                    r * cos(a + das) * cos(b + db),
                    r * cos(a + das) * sin(b + db),
                    r * sin(a + das));
                // glColor3f(1,1,0);
            //P4
                glTexCoord2f(ii,jj+djjs);
                glVertex3f(
                    r * cos(a + das) * cos(b),
                    r * cos(a + das) * sin(b),
                    r * sin(a + das));
                // glColor3f(1,0,1);
            glEnd();
        }
    }
    glPopMatrix();
    glFlush();
    glutSwapBuffers();
}


GLvoid reshape(GLint w, GLint h)
{
  glViewport(0, 0, w, h);
}

GLvoid key_press(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 'f':
      printf("fps: %g\n",fps);
      break;
    case 'm':
      mirror = !mirror;
      break;
    case KEY_ESCAPE:
    //   cvReleaseCapture(&g_Capture);
      glutDestroyWindow(g_hWindow);
      exit(0);
      break;
  }
  glutPostRedisplay();
}


GLvoid idle()
{
  // start timer
    double start_seconds = current_time_in_seconds();

    cap_center >> image_c;
    cap_left >> image_l;
    cap_right >> image_r;

    image=panorama->Blend_tr(image_l,image_c,image_r);

    char * buffer = new char[image.cols*image.rows*3];
    int step_img = image.step; 

    int height = image.rows;
    int width = image.cols; 
    int channels = 3;

    char * data_img  = (char *)image.data;
    // memcpy version below seems slightly faster
    for(int i=0;i<height;i++)
    {
      memcpy(&buffer[i*width*channels],&(data_img[i*step_img]),width*channels);
    }

    // Create Texture
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        image.cols,
        image.rows,
        0,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        buffer);

    // Clean up buffer
    delete[] buffer;

    // Update display
    glutPostRedisplay();

    double stop_seconds = current_time_in_seconds();
    fps = 0.9*fps + 0.1*1.0/(stop_seconds-start_seconds);
}



void mouseMotionCB(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}

void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
            glutWarpPointer(0,0);
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}