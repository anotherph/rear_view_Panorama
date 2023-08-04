// Modified from "Video Texture" code
// Copyright (C) 2009  Arsalan Malik (arsalank2@hotmail.com)
//                                                                            
// On Mac OS X, compile with:
// g++ -o VideoTexture VideoTexture.cpp -framework OpenGL -framework Glut -I
// /usr/local/include/opencv/ $(pkg-config --libs opencv)
//////////////////////////////////////////////////////////////////////////////
// Modified from "Video Texture" code as C++ language
// url :: https://gist.github.com/Ashwinning/baeb0835624fedc2e5b809d42417b70e

// Open CV includes
// #include <cv.h>
// #include <highgui.h>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp> 

#include "./pano_str_h.h"

// Standard includes
// #include <stdio.h>
#include <iostream>
#include <string.h>
#include <assert.h>

// OpenGL/Glut includes
#include <GL/glut.h>

// Timing includes
#include <sys/time.h>

#define KEY_ESCAPE 27 // 'ESC'

cv::VideoCapture cap_center(4), cap_left(16), cap_right(10);
cv::Mat image; // panorama image
cv::Mat image_c, image_l, image_r; 
GLint g_hWindow;

// // Frame size
// int frame_width  = 640;
// int frame_height = 480;
int frame_width  = 1920;
int frame_height = 1080;

// current frames per second, slightly smoothed over time
double fps;
// show mirror image
bool mirror = true;

// Return current time in seconds
double current_time_in_seconds();
// Initialize glut window
GLvoid init_glut();
// Glut display callback, draws a single rectangle using video buffer as
// texture
GLvoid display();
// Glut reshape callback
GLvoid reshape(GLint w, GLint h);
// Glut keyboard callback
GLvoid key_press (unsigned char key, GLint x, GLint y);
// Glut idle callback, fetches next video frame
GLvoid idle();

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
  glutIdleFunc(idle);
}

GLvoid display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  // These are necessary if using glTexImage2D instead of gluBuild2DMipmaps
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  

  // Set Projection Matrix
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  // gluOrtho2D(0, frame_width, frame_height, 0); // setting the coordinate, default is set to be "glOrtho(-1,1,-1,1,-1,1)"

  // Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glPushMatrix(); //(2)
  // glRotatef(150, 1, 0, 0);
  // glRotatef(60, 0, 1, 0);
  // glRotatef(20, 0, 0, 1);
  glRotatef(90, 1, 0, 0);
  glRotatef(0, 0, 1, 0);
  glRotatef(90, 0, 0, 1);

    // ############### hemisphere+cylinder ###############
    float r = 0.8; 
    float rp1, rp2; 
    float pi = 3.141592;
    float a_i=0.5;int num_i=20;
    // float a_j=0.5;int num_j=50;
    float a_jc=0.28; // maximum angle to draw cylinder in terms of spherical coordinate is 0.25 
    float a_js=0.25; int num_j=num_i; // angle for sphere, a_js is recommand to be mode(0.5/a_js)=0 
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
            // float ii = i/a_i*0.5+0.5; 
            // float jj = j/(a_jc+a_js)+0.5;
            float ii = i+a_i; ii = ii/(2*a_i);
            float jj = j+a_jc; jj = jj/(a_jc+a_js); 
            rp1=r / cos (a);
            rp2=r / cos (a+dac); 
            
            // std::cout<<"X:"<<ii<<std::endl;
            // std::cout<<"Y:"<<jj<<std::endl;

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
                // std::cout<<"p1:"<<ii<< " " <<jj<<std::endl;
                // std::cout<< rp1 * cos(a) * cos(b) <<" "<< rp1 * cos(a) * sin(b) <<" "<<rp1 * sin(a)<<std::endl;
                
            //P2
                glTexCoord2f(ii+dii, jj);
                glVertex3f(
                    rp1 * cos(a) * cos(b + db),
                    rp1 * cos(a) * sin(b + db),
                    rp1 * sin(a));
                // glColor3f(1,0,0);
                // std::cout<<"p2:"<<ii+dii<< " " <<jj<<std::endl;
            //P3
                glTexCoord2f(ii+dii, jj+djjc);
                glVertex3f(
                    rp2 * cos(a + dac) * cos(b + db),
                    rp2 * cos(a + dac) * sin(b + db),
                    rp2 * sin(a + dac));
                // glColor3f(1,1,0);
                // std::cout<<"p3:"<<ii+dii<< " " <<jj+djjc<<std::endl;
            //P4
                glTexCoord2f(ii,jj+djjc);
                glVertex3f(
                    rp2 * cos(a + dac) * cos(b),
                    rp2 * cos(a + dac) * sin(b),
                    rp2 * sin(a + dac));
                // glColor3f(1,0,1);
                // std::cout<<"p4:"<<ii<< " " <<jj+djjc<<std::endl;
            glEnd();
        }

        for (float j = 0; j <a_js-djs; j += djs) //vertical, hemisphere
        {
            float b = i * pi;      //0     to  2pi
            float a = j * pi;
            // float ii = i/a_i*0.5+0.5; 
            // // float jj = j/a_j*0.5+0.5;
            // float jj = j/(a_jc+a_js)+0.5;
            float ii = i+a_i; ii = ii/(2*a_i);
            float jj = j+a_jc; jj = jj/(a_jc+a_js); 

            // std::cout<<"X:"<<ii<<std::endl;
            // std::cout<<"Y:"<<jj<<std::endl;

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

  // Capture next frame, this will almost always be the limiting factor in the
  // framerate, my webcam only gets ~15 fps
  // IplImage * image = cvQueryFrame(g_Capture);

    Panorama *panorama = new Panorama(); 

    cap_center >> image_c;
    cap_left >> image_l;
    cap_right >> image_r;

    image = panorama->makePano2D(image_l,image_c,image_r);

  // Of course there are faster ways to do this with just opengl but this is to
  // demonstrate filtering the video before making the texture
  // if(mirror)
  // {
  //     cv::cvFlip(image, NULL, 1);
  // }

  // Image is memory aligned which means we there may be extra space at the end
  // of each row. gluBuild2DMipmaps needs contiguous data, so we buffer it here
  // char * buffer = new char[image->width*image->height*image->nChannels];
  char * buffer = new char[image.cols*image.rows*3];
  // int step     = image->widthStep;
  int step_img = image.step; 
  // int height   = image->height;
  // int width    = image->width;
  int height = image.rows;
  int width = image.cols; 
  int channels = 3;
  // char * data  = (char *)image->imageData;
  char * data_img  = (char *)image.data;
  // memcpy version below seems slightly faster
  //for(int i=0;i<height;i++)
  //for(int j=0;j<width;j++)
  //for(int k=0;k<channels;k++)
  //{
  //  buffer[i*width*channels+j*channels+k] = data[i*step+j*channels+k];
  //}
  for(int i=0;i<height;i++)
  {
    // memcpy(&buffer[i*width*channels],&(data[i*step]),width*channels);
    memcpy(&buffer[i*width*channels],&(data_img[i*step_img]),width*channels);
  }

  // Create Texture
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      // image->width,
      // image->height,
      image.cols,
      image.rows,
      0,
      GL_BGR,
      GL_UNSIGNED_BYTE,
      buffer);

  // Clean up buffer
  delete[] buffer;
//   delete[] panorama; 

  // Update display
  glutPostRedisplay();

  double stop_seconds = current_time_in_seconds();
  fps = 0.9*fps + 0.1*1.0/(stop_seconds-start_seconds);
}

int main(int argc, char* argv[])
{

    // VideoCapture cap_center(4); // center cam
    int width = 1920;
    int height = 1080; 
    cap_center.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap_center.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap_left.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap_left.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap_right.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap_right.set(cv::CAP_PROP_FRAME_HEIGHT, height);

  // capture properties
  // frame_height = (int)cvGetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_HEIGHT);
  // frame_width  = (int)cvGetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_WIDTH);
  int frame_height = height;
  int frame_width = width; 

  // Create GLUT Window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(frame_width, frame_height);

  g_hWindow = glutCreateWindow("Video Texture");

  // Initialize OpenGL
  init_glut();

  glutMainLoop();

  return 0;
}