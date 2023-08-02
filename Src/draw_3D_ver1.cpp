// https://gamedev.stackexchange.com/questions/114412/how-to-get-uv-coordinates-for-sphere-cylindrical-projection
// u, v calculation 

//https://stackoverflow.com/questions/46354887/read-color-bytes-rgb-from-a-bitmap
// change the order of RGB...

#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <cstring>


typedef unsigned int int32;
typedef short int16;
typedef unsigned char byte;

// global variables
void *font = GLUT_BITMAP_8_BY_13;
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
int drawMode = 0;

const float CAMERA_DISTANCE = 10.0f;

#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}

void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
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

void LoadBmp(const char *fileName, byte **pRaster, int32 *width, int32 *height, int32 *bytesPerPixel)
{
    //Open the file for reading in binary mode
    FILE *imageFile = fopen(fileName, "rb");
    //Read data offset
    int32 dataOffset;
    fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
    fread(&dataOffset, 4, 1, imageFile);
    //Read width
    fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, imageFile);
    // *width = 4800; 
    //Read height
    fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, imageFile);
    //Read bits per pixel
    int16 bitsPerPixel;
    fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bitsPerPixel, 2, 1, imageFile);
    //Allocate a pixel array
    *bytesPerPixel = ((int32)bitsPerPixel) / 8;

    // byte *pRaster;

    //Rows are stored bottom-up
    //Each row is padded to be a multiple of 4 bytes. 
    //We calculate the padded row size in bytes
    int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f))*(*bytesPerPixel);
    //We are not interested in the padded bytes, so we allocate memory just for
    //the pixel data
    int unpaddedRowSize = (*width)*(*bytesPerPixel);
    
    //Total size of the pixel data in bytes
    int totalSize = unpaddedRowSize*(*height);
    *pRaster = (byte*)malloc(totalSize);
    //Read the pixel data Row by Row.
    //Data is padded and stored bottom-up
    int i = 0;
    //point to the last row of our pixel array (unpadded)
    byte *currentRowPointer = *pRaster+((*height-1)*unpaddedRowSize);
    for (i = 0; i < *height; i++)
    {
        // std::cout<<currentRowPointer<<std::endl;
        // for (byte *p = currentRowPointer;p<currentRowPointer+unpaddedRowSize-3;p+=3)
        // {
        //     std::cout<<p<<std::endl;
        //     byte b = *p;
        //     *p=*(p+2);
        //     *(p+2)=b;
        // } // i dont know how to do.. just crying T.T 
        //put file cursor in the next row from top to bottom
        fseek(imageFile, dataOffset+(i*paddedRowSize), SEEK_SET);
        //read only unpaddedRowSize bytes (we can ignore the padding bytes)
        fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
        //point to the next row (from bottom to top)
        currentRowPointer -= unpaddedRowSize;
    }
    fclose(imageFile);
}

void displayMe(void) // draw sphere
{
    // load the bmp images
    int32 width;
    int32 height; 
    int32 bytesPerPixel;
    byte *data;
    GLint EnvMode = GL_REPLACE; // GL_REPLACE & GL_MOCULATE only , GL_ADD is not avaliavble 
    GLint TexFilter = GL_NEAREST;  //
    // GLint TexFilter = GL_LINEAR;  //
    GLuint myT;
    
    LoadBmp("/home/jekim/workspace/rear_view_Panorama/Img/pano.bmp", &data, &width, &height, &bytesPerPixel);
    // LoadBmp("/home/jekim/workspace/opengl_prac/img.bmp", &data, &width, &height, &bytesPerPixel);
    // LoadBmp("/home/jekim/Downloads/original.bmp", &data, &width, &height, &bytesPerPixel);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
    
    glPushMatrix(); //(2)

    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);

    // glRotatef(90, 1, 0, 0);
    // glRotatef(-0, 0, 1, 0);
    glRotatef(-90, 0, 0, 1);
    // glTranslatef(-0.5, 0, 0);
    // std::cout<<"X:"<<cameraAngleX/10<<std::endl;

    // glRotatef(100, 1, 0, 0);
    // glRotatef(20, 0, 1, 0);
    // glRotatef(-90, 0, 0, 1);
    // glTranslatef(-0.5, 0, 0);

    // texture setting 

    glGenTextures(1, &myT);
    glBindTexture(GL_TEXTURE_2D,myT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,0, GL_RGB,  GL_UNSIGNED_BYTE, data);
    free(data);
    //     // 텍스처 환경 설정
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, EnvMode);
    //     // 텍스처 필터 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TexFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TexFilter);
    glEnable(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    glShadeModel(GL_SMOOTH);
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // // ############### triangular corn ###############
    // // 아랫면 흰 바닥
    // glBegin(GL_QUADS);
    // // glColor3f(1,1,1);
    // glTexCoord2f(0.0, 1.0);
    // glVertex2f(-0.5, 0.5);
    // glTexCoord2f(1.0, 1.0);
    // glVertex2f(0.5, 0.5);
    // glTexCoord2f(1.0, 0.0);
    // glVertex2f(0.5, -0.5);
    // glTexCoord2f(0.0, 0.0);
    // glVertex2f(-0.5, -0.5);
    // glEnd();
    // // 위쪽 빨간 변
    // glBegin(GL_TRIANGLE_FAN);
    // // glColor3f(1,0,0);
    // glTexCoord2f(0.5, 0.5);
    // glVertex3f(0.0, 0.0, -0.8);
    // glTexCoord2f(1.0, 1.0);
    // glVertex2f(0.5, 0.5);
    // glTexCoord2f(0.0, 1.0);
    // glVertex2f(-0.5, 0.5);
    // // 왼쪽 노란 변
    // // glColor3f(1,1,0);
    // glTexCoord2f(0.0, 0.0);
    // glVertex2f(-0.5, -0.5);
    // // 아래쪽 초록 변
    // // glColor3f(0,1,0);
    // glTexCoord2f(1.0, 0.0);
    // glVertex2f(0.5, -0.5);
    // // 오른쪽 파란 변
    // // glColor3f(0,0,1);
    // glTexCoord2f(1.0, 1.0);
    // glVertex2f(0.5, 0.5);
    // glEnd();
    // glPopMatrix();
    // glFlush();

    // // ############### sphere ###############
    // float r = 1; 
    // float pi = 3.141592;
    // float a_i=0.5;int num_i=10;
    // float a_j=0.2;int num_j=10;
    // float di = 2*a_i/num_i;
    // float dj = 2*a_j/num_j;
    // float db = di * pi;
    // float da = dj * pi;
    // float dii=1/float(num_i); 
    // float djj=1/float(num_j);

    // for (float i = -a_i; i < a_i; i += di) //horizonal
    // {
    //     for (float j = -a_j; j <a_j*0.95; j += dj) //vertical
    //     {
    //         float b = i * pi;      //0     to  2pi
    //         float a = j * pi;
    //         float ii = i/a_i*0.5+0.5; 
    //         float jj = j/a_j*0.5+0.5;
    //         std::cout<<"X:"<<ii<<std::endl;
    //         std::cout<<"X:"<<jj<<std::endl;

    //         //normal
    //         glNormal3f(
    //             cos(a + da / 2) * cos(b + db / 2),
    //             cos(a + da / 2) * sin(b + db / 2),
    //             sin(a + da / 2));

    //         glBegin(GL_QUADS);
    //         //P1
    //             glTexCoord2f(ii, jj);
    //             glVertex3f(
    //                 r * cos(a) * cos(b),
    //                 r * cos(a) * sin(b),
    //                 r * sin(a));
    //             // glColor3f(0,0,0);
                
    //         //P2
    //             glTexCoord2f(ii+dii, jj);
    //             glVertex3f(
    //                 r * cos(a) * cos(b + db),
    //                 r * cos(a) * sin(b + db),
    //                 r * sin(a));
    //             // glColor3f(1,0,0);
    //         //P3
    //             glTexCoord2f(ii+djj, jj+djj);
    //             glVertex3f(
    //                 r * cos(a + da) * cos(b + db),
    //                 r * cos(a + da) * sin(b + db),
    //                 r * sin(a + da));
    //             // glColor3f(1,1,0);
    //         //P4
    //             glTexCoord2f(ii,jj+djj);
    //             glVertex3f(
    //                 r * cos(a + da) * cos(b),
    //                 r * cos(a + da) * sin(b),
    //                 r * sin(a + da));
    //             // glColor3f(1,0,1);
    //         glEnd();
    //     }
    // }
    // glPopMatrix();
    // glFlush();
    // glDeleteTextures(1,&myT);

    // ############### hemisphere+cylinder ###############
    float r = 0.8; 
    float rp1, rp2; 
    float pi = 3.141592;
    float a_i=0.5;int num_i=20;
    // float a_j=0.5;int num_j=50;
    float a_jc=0.25; // maximum angle to draw cylinder in terms of spherical coordinate is 0.25 
    float a_js=0.2; int num_j=num_i; // angle for sphere, a_js is recommand to be mode(0.5/a_js)=0 
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
    glDeleteTextures(1,&myT);
}

int main(int argc, char** argv)
{   

    glutInit(&argc, argv); // (1)
    // glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode
    glutInitDisplayMode(GLUT_SINGLE); // (2)
    glutInitWindowSize(1600, 700); // (3)
    glutInitWindowPosition(400, 400); //(4)
    glutCreateWindow("Hello world :D"); // (5)
    // glutReshapeFunc(reshapeCB);
    glutDisplayFunc(displayMe); // (6)
    glutTimerFunc(33, timerCB, 33); // (7) , this line make the image flink... 
    glutMouseFunc(mouseCB); // (8)
    glutMotionFunc(mouseMotionCB); //(9)
    glutMainLoop(); // the last ... 
    
    return 0;
}
