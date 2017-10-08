#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <GL/glut.h>
#include "Matrix.h"
#include "WaveFrontOBJ.h"

// 'cameras' stores infomation of 5 cameras.
float cameras[5][9] =
{
	{ 28, 18, 28, 0, 2, 0, 0, 1, 0 },
	{ 28, 18, -28, 0, 2, 0, 0, 1, 0 },
	{ -28, 18, 28, 0, 2, 0, 0, 1, 0 },
	{ -12, 12, 0, 0, 2, 0, 0, 1, 0 },
	{ 0, 100, 0, 0, 0, 0, 1, 0, 0 }
};
int cameraCount = sizeof(cameras) / sizeof(cameras[0]);

int cameraIndex;
vector<Matrix> wld2cam, cam2wld;

// Variables for 'cow' object.
Matrix cow2wld;
WaveFrontOBJ* cow;

// Variables for 'bunny' object.
Matrix bunny2wld;
WaveFrontOBJ* bunny;

unsigned floorTexID;

int frame = 0;
int width, height;
int oldX, oldY;

void translate();
void rotate();

int selectMode;
bool picking = false;

/*******************************************************************/
//(PA #2,#3) 수행한 내용 추가
/*******************************************************************/

bool keym = false;
bool keyv = false;
int keystate = 0;  // x?y?z축인지 결정하기 위해서 설정
GLfloat theta = 0.5; // 각의 크기
bool keyr = false;
bool keyn, keyf = false; //face normal vector vertex normal vector를 켜고 끄는 키이다.
int len = 0; //얼만큼 translate할지 결정하는 길이
int dragon = 0; //drag가 on 되어있다는 뜻 (m모드에서)
int translatekey = 0; // xy평면? z평면?
int lenX = 0; //v모드에서 얼만큼 translate할지 결정하는 길이
int lenY = 0; //v모드에서 얼만큼 translate할지 결정하는 길이 (y기준)
bool currentobj = true; //true면 소, false면 bunny
bool shademodel = true; //s키
bool light1 = false;
bool light2 = false;
bool light3 = false; // 각각 light들을 켜고 끄는데 사용된다.
bool bunnyv, bunnyf, cowv, cowf = false; //bunny에 대해 normal vector를 표현할건지, cow에 대해 표현할 건지 알려주는 변수

//------------------------------------------------------------------------------
void munge(int x, float& r, float& g, float& b)
{
	r = (x & 255) / float(255);
	g = ((x >> 8) & 255) / float(255);
	b = ((x >> 16) & 255) / float(255);
}





//------------------------------------------------------------------------------
int unmunge(float r, float g, float b)
{
	return (int(r) + (int(g) << 8) + (int(b) << 16));
}





/*********************************************************************************
* Draw x, y, z axis of current frame on screen.
* x, y, and z are corresponded Red, Green, and Blue, resp.
**********************************************************************************/
void drawAxis(float len)
{
	glDisable(GL_LIGHTING);		// Lighting is not needed for drawing axis.
	glBegin(GL_LINES);			// Start drawing lines.
	glColor3d(1, 0, 0);			// color of x-axis is red.
	glVertex3d(0, 0, 0);
	glVertex3d(len, 0, 0);		// Draw line(x-axis) from (0,0,0) to (len, 0, 0). 
	glColor3d(0, 1, 0);			// color of y-axis is green.
	glVertex3d(0, 0, 0);
	glVertex3d(0, len, 0);		// Draw line(y-axis) from (0,0,0) to (0, len, 0).
	glColor3d(0, 0, 1);			// color of z-axis is  blue.
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, len);		// Draw line(z-axis) from (0,0,0) - (0, 0, len).
	glEnd();					// End drawing lines.
	glEnable(GL_LIGHTING);
}





void InitCamera(){

	// initialize camera frame transforms.
	for (int i = 0; i < cameraCount; i++)
	{
		float* c = cameras[i];													// 'c' points the coordinate of i-th camera.
		wld2cam.push_back(Matrix());											// Insert {0} matrix to wld2cam vector.
		glPushMatrix();															// Push the current matrix of GL into stack.

		glLoadIdentity();														// Set the GL matrix Identity matrix.
		gluLookAt(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8]);		// Setting the coordinate of camera.
		glGetFloatv(GL_MODELVIEW_MATRIX, wld2cam[i].matrix());					// Read the world-to-camera matrix computed by gluLookAt.
		cam2wld.push_back(wld2cam[i].inverse());								// Get the camera-to-world matrix.

		glPopMatrix();															// Transfer the matrix that was pushed the stack to GL.
	}
	cameraIndex = 0;
}





void drawCamera()
{
	int i;
	// set viewing transformation.
	glLoadMatrixf(wld2cam[cameraIndex].matrix());

	// draw cameras.
	for (i = 0; i < (int)wld2cam.size(); i++)
	{
		if (i != cameraIndex)
		{
			glPushMatrix();													// Push the current matrix on GL to stack. The matrix is wld2cam[cameraIndex].matrix().
			glMultMatrixf(cam2wld[i].matrix());								// Multiply the matrix to draw i-th camera.
			if (selectMode == 0)											// selectMode == 1 means backbuffer mode.
			{
				drawAxis(5);												// Draw x, y, and z axis.
			}
			else
			{
				float r, g, b;
				glDisable(GL_LIGHTING);										// Disable lighting in backbuffer mode.
				munge(i + 1, r, g, b);										// Match the corresponding (i+1)th color to r, g, b. You can change the color of camera on backbuffer.
				glColor3f(r, g, b);											// Set r, g, b the color of camera.
			}

			glutSolidSphere(1.0, 10, 10);

			glPopMatrix();													// Call the matrix on stack. wld2cam[cameraIndex].matrix() in here.
		}
	}
}





/*********************************************************************************
* Draw 'cow' object.
**********************************************************************************/
void InitCow(){
	cow = new WaveFrontOBJ("cow_pa4.obj");

	glPushMatrix();											// Push the current matrix of GL into stack.
	glLoadIdentity();										// Set the GL matrix Identity matrix.
	glTranslatef(0, -cow->bbmin.pos.y, -8);					// Set the location of cow.
	glRotatef(-90, 0, 1, 0);								// Set the direction of cow. These information are stored in the matrix of GL.
	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());		// Read the modelview matrix about location and direction set above, and store it in cow2wld matrix.
	glPopMatrix();											// Pop the matrix on stack to GL.

}





void drawCow()
{
	glPushMatrix();											// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing cow.

	// The information about location of cow to be drawn is stored in cow2wld matrix.
	// If you change the value of the cow2wld matrix or the current matrix, cow would rotate or move.
	glMultMatrixf(cow2wld.matrix());

	if (selectMode == 0)									// selectMode == 1 means backbuffer mode.
	{
		drawAxis(5);										// Draw x, y, and z axis.
	}
	else
	{
		float r, g, b;
		glDisable(GL_LIGHTING);								// Disable lighting in backbuffer mode.
		munge(32, r, g, b);									// Match the corresponding constant color to r, g, b. You can change the color of camera on backbuffer
		glColor3f(r, g, b);
	}
	
	cow->Draw();
	/*******************************************************************/
	//(PA #4) : cow object의 normal을 그리는 함수를 추가하십시오.
	/*******************************************************************/
	if (cowf == true)
		cow->Draw_FN();
	if (cowv == true)
		cow->Draw_VN();

	//f키가 켜져있을 때 object picking이 cow로 켜져있으면 face normal vector켜주고,
	//v키가 켜져있으면 vertex normal vector 켜준다.
	glPopMatrix();											// Pop the matrix in stack to GL. Change it the matrix before drawing cow.
}





void Initbunny(){
	bunny = new WaveFrontOBJ("bunny_pa4.obj");

	glPushMatrix();											// Push the current matrix of GL into stack.
	glLoadIdentity();										// Set the GL matrix Identity matrix.
	glTranslatef(0, -bunny->bbmin.pos.y, 8);					// Set the location of bunny.
	glRotatef(180, 0, 1, 0);								// Set the direction of bunny. These information are stored in the matrix of GL.
	glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());		// Read the modelview matrix about location and direction set above, and store it in bunny2wld matrix.
	glPopMatrix();											// Pop the matrix on stack to GL.

}





void drawbunny()
{
	glPushMatrix();		// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing bunny.

	// The information about location of bunny to be drawn is stored in bunny2wld matrix.
	// If you change the value of the bunny2wld matrix or the current matrix, bunny would rotate or move.
	glMultMatrixf(bunny2wld.matrix());

	if (selectMode == 0)									// selectMode == 1 means backbuffer mode.
	{
		drawAxis(8);										// Draw x, y, and z axis.
	}
	else
	{
		float r, g, b;
		glDisable(GL_LIGHTING);								// Disable lighting in backbuffer mode.
		munge(33, r, g, b);									// Match the corresponding constant color to r, g, b. You can change the color of camera on backbuffer
		glColor3f(r, g, b);
	}
	
	bunny->Draw();// Draw the bunny 
	/*******************************************************************/
	//(PA #4) : bunny object의 normal을 그리는 함수를 추가하십시오.
	/*******************************************************************/
	if (bunnyf == true)
		bunny->Draw_FN();
	if (bunnyv == true)
		bunny->Draw_VN();
	//f키가 켜져있을 때 object picking이 bunny로 켜져있으면 face normal vector켜주고,
	//v키가 켜져있으면 vertex normal vector 켜준다.
	glPopMatrix();											// Pop the matrix in stack to GL. Change it the matrix before drawing bunny.
}





/*********************************************************************************
* Draw floor on 3D plane.
**********************************************************************************/
void drawFloor()
{
	if (frame == 0)
	{
		// Initialization part.
		// After making checker-patterned texture, use this repetitively.

		// Insert color into checker[] according to checker pattern.
		const int size = 8;
		unsigned char checker[size*size * 3];
		for (int i = 0; i < size*size; i++)
		{
			if (((i / size) ^ i) & 1)
			{
				checker[3 * i + 0] = 100;
				checker[3 * i + 1] = 100;
				checker[3 * i + 2] = 100;
			}
			else
			{
				checker[3 * i + 0] = 200;
				checker[3 * i + 1] = 200;
				checker[3 * i + 2] = 200;
			}
		}

		// Make texture which is accessible through floorTexID. 
		glGenTextures(1, &floorTexID);
		glBindTexture(GL_TEXTURE_2D, floorTexID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, checker);
	}

	glDisable(GL_LIGHTING);

	// Set background color.
	if (selectMode == 0)
		glColor3d(0.35, 0.2, 0.1);
	else
	{
		// In backbuffer mode.
		float r, g, b;
		munge(34, r, g, b);
		glColor3f(r, g, b);
	}

	// Draw background rectangle.
	glBegin(GL_POLYGON);
	glVertex3f(-2000, -0.2, -2000);
	glVertex3f(-2000, -0.2, 2000);
	glVertex3f(2000, -0.2, 2000);
	glVertex3f(2000, -0.2, -2000);
	glEnd();


	// Set color of the floor.
	if (selectMode == 0)
	{
		// Assign checker-patterned texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorTexID);
	}
	else
	{
		// Assign color on backbuffer mode.
		float r, g, b;
		munge(35, r, g, b);
		glColor3f(r, g, b);
	}

	// Draw the floor. Match the texture's coordinates and the floor's coordinates resp. 
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	glVertex3d(-12, -0.1, -12);		// Texture's (0,0) is bound to (-12,-0.1,-12).
	glTexCoord2d(0, 1);
	glVertex3d(-12, -0.1, 12);		// Texture's (0,1) is bound to (-12,-0.1,12).
	glTexCoord2d(1, 1);
	glVertex3d(12, -0.1, 12);		// Texture's (1,1) is bound to (12,-0.1,12).
	glTexCoord2d(1, 0);
	glVertex3d(12, -0.1, -12);		// Texture's (1,0) is bound to (12,-0.1,-12).

	glEnd();

	if (selectMode == 0)
	{
		glDisable(GL_TEXTURE_2D);
		drawAxis(5);				// Draw x, y, and z axis.
	}
}





void Lighting()
{
	/*******************************************************************/
	//(PA #4) : 다양한 광원을 구현하십시오.
	//1. Point light / Directional light / Spotlight를 서로 다른 색으로 구현하십시오.
	//2. 광원의 위치를 구(sphere)로 표현하십시오.
	//3. Directional light / Spotlight의 경우 빛의 진행방향을 선분으로 표현하십시오.
	/*******************************************************************/

	float point_pos[] = { 0.0,0.0,0.0,1.0 };
	float point_amb[] = { 0.0,1.0,1.0,1.0 };
	float dir_pos[] = { 1.0,1.0,0.0,0.0 };
	float point_color[] = { 0.0,6.0,6.0,6.0 };
	float dir_color[] = { 6.0,6.0,0.0,6.0 };
	float dir_amb[] = { 1.0,1.0,0.0,1.0 };
	float spot_pos[] = { 0.0,10.0,-5.0,1.0 };
	float spot_dir[] = { 0.0,-1.0,1.0 };
	float spot_color[] = { 6.0,0.0,6.0,6.0 };
	float spot_amb[] = { 1.0,0.0,1.0,1.0 };
	float spot_spec[] = { 9.0,0.0,9.0,9.0 };
	float dir_spec[] = { 9.0,9.0,0.0,9.0 };
	float point_spec[] = { 0.0,9.0,9.0,9.0 };


	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, point_color);
	glLightfv(GL_LIGHT1, GL_AMBIENT, point_amb);
	glLightfv(GL_LIGHT1, GL_POSITION, point_pos);
	glLightfv(GL_LIGHT1, GL_SPECULAR, point_spec);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, dir_color);
	glLightfv(GL_LIGHT2, GL_AMBIENT, dir_amb);
	glLightfv(GL_LIGHT2, GL_POSITION, dir_pos);
	glLightfv(GL_LIGHT2, GL_SPECULAR, dir_spec);
	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 5.0f);
	glLightfv(GL_LIGHT3, GL_POSITION, spot_pos);
	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spot_dir);
	glLightfv(GL_LIGHT3, GL_SPECULAR, spot_spec);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, spot_color);
	glLightfv(GL_LIGHT3, GL_AMBIENT, spot_amb);

	//1번 광원은 point, 2번 광원은 direction, 3번 광원은 spotlight

	glPushMatrix();
	glTranslatef(0.0,0.0,0.0);
	glColor3f(1.0, 1.0, 1.0);
	glMaterialf(GL_FRONT, GL_EMISSION, 1000);
	glutSolidSphere(0.5f, 50, 10);
	glTranslatef(0.0, 10.0, -5.0);
	glutSolidSphere(0.5f, 50, 10);
	glPopMatrix();

	//각 광원이 위치한 곳에 구 두개를 그려주었다.(direction 제외)

	if (light3 == true) {
		glBegin(GL_LINES);
		glColor3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 10.0, -5.0);
		glVertex3f(0.0, 3.0, 2.0);
		glEnd();
	}
	if (light2 == true) {
		glColor3d(0,0,0);
		glBegin(GL_LINES);
		glVertex3f(30.0, 30.0, 0.0);
		glVertex3f(10.0, 10.0, 0.0);
		glEnd();
	}
	//각각의 방향을 선으로 표시해주었다.
}





/*********************************************************************************
* Call this part whenever display events are needed.
* Display events are called in case of re-rendering by OS. ex) screen movement, screen maximization, etc.
* Or, user can occur the events by using glutPostRedisplay() function directly.
* this part is called in main() function by registering on glutDisplayFunc(display).
**********************************************************************************/
void display()
{
	if (selectMode == 0)
		glClearColor(0, 0.6, 0.8, 1);								// Clear color setting
	else
		glClearColor(0, 0, 0, 1);									// When the backbuffer mode, clear color is set to black

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear the screen

	if (shademodel)
		glShadeModel(GL_FLAT);										// Set Flat Shading
	else
		glShadeModel(GL_SMOOTH);

	drawCamera();													// and draw all of them.
	drawFloor();													// Draw floor.

	drawCow();														// Draw cow.
	drawbunny();

	Lighting();

	glFlush();

	// If it is not backbuffer mode, swap the screen. In backbuffer mode, this is not necessary because it is not presented on screen.
	if (selectMode == 0)
		glutSwapBuffers();

	frame++;
}





/*********************************************************************************
* Call this part whenever size of the window is changed.
* This part is called in main() function by registering on glutReshapeFunc(reshape).
**********************************************************************************/
void reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);            // Select The Projection Matrix
	glLoadIdentity();                       // Reset The Projection Matrix
	float aspect = width / float(height);
	gluPerspective(45, aspect, 1, 1024);

	glMatrixMode(GL_MODELVIEW);             // Select The Modelview Matrix
	glLoadIdentity();                       // Reset The Projection Matrix
}





//------------------------------------------------------------------------------
void initialize()
{
	// Set up OpenGL state
	glEnable(GL_DEPTH_TEST);         // Enables Depth Testing
	// Initialize the matrix stacks
	reshape(width, height);
	// Define lighting for the scene
	float light_pos[] = { 1.0, 1.0, 1.0, 0.0 };
	float light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	float light_diffuse[] = { 0.9, 0.9, 0.9, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);

	InitCamera();
	InitCow();
	Initbunny();
}





/*********************************************************************************
* Call this part whenever mouse button is clicked.
* This part is called in main() function by registering on glutMouseFunc(onMouseButton).
**********************************************************************************/
void onMouseButton(int button, int state, int x, int y)
{
	y = height - y - 1;
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			printf("Left mouse click at (%d, %d)\n", x, y);

			// After drawing object on backbuffer, you can recognize which object is selected by reading pixel of (x, y).
			// Change the value of selectMode to 1, then draw the object on backbuffer when display() function is called. 
			selectMode = 1;
			display();
			glReadBuffer(GL_BACK);
			unsigned char pixel[3];
			glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
			int pixel_value = unmunge(pixel[0], pixel[1], pixel[2]);
			printf("pixel = %d\n", pixel_value);

			if (picking) {

				/*******************************************************************/
				//(PA #3) 클릭한 좌표의 픽셀 값에 따라 다른 오브젝트를 선택하게 구현.
				/*******************************************************************/
				if (pixel_value == 32)
					currentobj = true; //32면 소
				else if (pixel_value == 33)
					currentobj = false; //33이면 bunny
				else
					printf("error\n"); //없음 err
				picking = false;
			}

			selectMode = 0;
			// Save current clicked location of mouse here, and then use this on onMouseDrag function. 
			oldX = x;
			oldY = y;
		}
	}

	glutPostRedisplay();
}





/*********************************************************************************
* Call this part whenever user drags mouse.
* Input parameters x, y are coordinate of mouse on dragging.
* Value of global variables oldX, oldY is stored on onMouseButton,
* Then, those are used to verify value of x - oldX,  y - oldY to know its movement.
**********************************************************************************/
void onMouseDrag(int x, int y)
{
	y = height - y - 1;
	printf("in drag (%d, %d)\n", x - oldX, y - oldY);

	/*******************************************************************/
	//(PA #3) : 마우스의 드래그 거리와 방향에 따라 오브젝트의 평행이동 거리 / 회전 정도를 결정하십시오.
	//1. 일반적인 평행이동의 경우 스크린상의 x축을 따라 드래그하면 평행이동하게 하십시오.
	//2. xy축 평면에 대해 이동하는 경우 x축, y축 이동을 따로 고려하십시오.
	//3. viewing space의 회전이동 문제의 경우 1.과 같은 방식으로 구현하십시오. 
	/*******************************************************************/
	if (keyv == true && translatekey == 1) { // xy평면
		lenX = x - oldX;
		lenY = y - oldY;
		translate();
	} //v모드에서
	if (keyv == true && translatekey == 2) { //z축
		lenX = x - oldX;
		translate();
	}//v모드

	if (keyv == true && keyr == true) { //viewing 좌표계 x축 기준 회전
		lenY = y - oldY;
		rotate();
	} // v모드

	if (keym == true) {
		if ((abs(x - oldX) < 10.0) /*&& (abs((double)y - oldY - (x - oldX)) > 10.0)*/) {
			//y평행이동(x가 조금 움직였으면), dragon을 2로 설정하고 바꾼 len을 기준으로 translate한다.
			dragon = 2;
			len = y - oldY;
			printf("%d", len);
			translate();
		}
		else if ((x - oldX <= 0 && y - oldY >= 0) || (x - oldX >= 0 && y - oldY <= 0)) {
			//z 평행이동(x와 y가 드래그된 방향이 서로 반대라면), dragon을 3로 설정하고 바꾼 len을 기준으로 translate한다.
			dragon = 3;
			len = x - oldX;
			translate();
		}
		else if ((x - oldX >= 0 && y - oldY >= 0) || (x - oldX <= 0 && y - oldY <= 0)) {
			//x 평행이동(x와 y가 드래그 된 방향이 똑같고 x가 많이 움직였다면), dragon을 1로 설정하고 바꾼 len을 기준으로 translate한다.
			dragon = 1;
			len = x - oldX;
			translate();
		}
		else
			dragon = 0;
	} //m모드
	oldX = x;
	oldY = y;
	glutPostRedisplay();
}





/*********************************************************************************
* Call this part whenever user types keyboard.
* This part is called in main() function by registering on glutKeyboardFunc(onKeyPress).
**********************************************************************************/
void onKeyPress(unsigned char key, int x, int y)
{//카메라 시점 변경
	if ((key >= '1') && (key <= '5'))
		cameraIndex = key - '1';

	//각각 해당하는 key를 눌렀을 때 변수를 바꿔줌

	if (key == 'n') {
		if (keyn == false) {
			if (currentobj) {
				cowv = true;
			}
			else {
				bunnyv = true;
			}
			keyn = true;
		}
		else {
			cowv = false;
			bunnyv = false;
			keyn = false;
		}
	}

	if (key == 'f') {
		if (keyf == false) {
			if (currentobj) {
				cowf = true;
			}
			else {
				bunnyf = true;
			}
			keyf = true;
		}
		else {
			cowf = false;
			bunnyf = false;
			keyf = false;
		}
	}

	if (key == 'p') {
		picking = true;
	}

	if (key == 'm') {
		if (keym == false) {
			keyv = false;
			keym = true;
		}
		else
			keym = false;
	}

	if (key == 'r') {
		if (keyr == false)
			keyr = true;
		else {
			keystate = 0;
			keyr = false;
		}
	}
	if (key == 'v') { //v모드일때는 m 끄고, m모드일때는 위에서 v꺼줌.
		if (keyv == false) {
			keyv = true;
			keym = false;
		}
		else {
			translatekey = 0;
			keyv = false;
		}
	}

	if (key == 'x') { //keystate는 m모드, translatekey는 v모드
		keystate = 1;
		translatekey = 1;
	}
	else if (key == 'y') {
		keystate = 2;
		translatekey = 1;
	}
	else if (key == 'z') {
		keystate = 3;
		translatekey = 2;
	}

	/*******************************************************************/
	//(PA #3) : 과제에서 제시하는 대로 키보드 입력에 대한 코드를 추가로 작성하십시오.
	/*******************************************************************/


	if (key == 's') {
		shademodel = !shademodel;
		cow->isFlat = shademodel;
		bunny->isFlat = shademodel;
	}

	glutPostRedisplay();
}






void SpecialKey(int key, int x, int y)
{
	/*******************************************************************/
	//(PA #4) : F1 / F2 / F3 버튼에 따라 서로 다른 광원이 On/Off 되도록 구현하십시오.
	/*******************************************************************/
	switch (key) { //key마다 glut에서 지원하는 key값과 비교해서 광원을 켜고 꺼주었다.
	case GLUT_KEY_F1:
		if (light1 == false) {
			glEnable(GL_LIGHT1);
			printf("1");
			light1 = true;
		}
		else {
			glDisable(GL_LIGHT1);
			light1 = false;
		}
		break;
	case GLUT_KEY_F2:
		if (light2 == false) {
			glEnable(GL_LIGHT2);
			light2 = true;
		}
		else {
			glDisable(GL_LIGHT2);
			light2 = false;
		}
		break;
	case GLUT_KEY_F3:
		if (light3 == false) {
			glEnable(GL_LIGHT3);
			light3 = true;
		}
		else {
			glDisable(GL_LIGHT3);
			light3 = false;
		}
		break;
	}
	glutPostRedisplay();
}






void rotate() {
	glPushMatrix();
	glLoadIdentity();
	//curobj가 true이면 소, false이면 bunny에 관련된 조건문들
	if (keym == true && keyr == true && currentobj == true) {
		glMultMatrixf(cow2wld.matrix());
		if (keystate == 1)
			glRotatef(theta, 1, 0, 0);
		else if (keystate == 2)
			glRotatef(theta, 0, 1, 0);
		else if (keystate == 3)
			glRotatef(theta, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
	}
	if (keym == true && keyr == true && currentobj == false) {
		glMultMatrixf(bunny2wld.matrix());
		if (keystate == 1)
			glRotatef(theta, 1, 0, 0);
		else if (keystate == 2)
			glRotatef(theta, 0, 1, 0);
		else if (keystate == 3)
			glRotatef(theta, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
	}

	//m모드에서 자기 좌표축 기준 회전


	if (keyv == true && keyr == true && currentobj == true) {
		glMultMatrixf(cow2wld.matrix()); //matrix를 로드 합니다.
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = cow2wld.m[i][j];
			}
		} //a matrix에 cow2wld matrix의 [0][0]~~[2][2]까지 복사합니다.
		Matrix b;
		a.m[3][3] = 1;
		a.set(a.inverse().m);//a의 inverse 를 b (viewing space의 x축 좌표)에 곱해줍니다. 그리고 나온 [0][0],[1][0],[2][0] 을 기준으로 rotate해 줍니다.
		b.m[0][0] = cam2wld[cameraIndex].m[0][0];
		b.m[1][0] = cam2wld[cameraIndex].m[0][1];
		b.m[2][0] = cam2wld[cameraIndex].m[0][2];
		a.set(a.multiply(b).m);
		glRotatef(lenY, a.m[0][0], a.m[1][0], a.m[2][0]);
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
	}

	if (keyv == true && keyr == true && currentobj == false) {
		glMultMatrixf(bunny2wld.matrix());
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = bunny2wld.m[i][j];
			}
		}
		Matrix b;
		a.m[3][3] = 1;
		a.set(a.inverse().m);
		b.m[0][0] = cam2wld[cameraIndex].m[0][0];
		b.m[1][0] = cam2wld[cameraIndex].m[0][1];
		b.m[2][0] = cam2wld[cameraIndex].m[0][2];
		a.set(a.multiply(b).m);
		glRotatef(lenY, a.m[0][0], a.m[1][0], a.m[2][0]);
		glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
	}

	//v모드에서 viewing 좌표계의 x축 기준 회전


	/*******************************************************************/
	//(PA #3) : 선택된 오브젝트의 회전을 구현하십시오.
	//1. modeling space 모드일 경우 'r'키로 회전상태가 ON 되었을 경우 선택된 축에 대해 자동으로 회전되게 하십시오.
	//2. viewing space 모드일 경우 x축을 기준으로 드래그 거리에 따라 회전하게 하십시오.
	/*******************************************************************/
	glPopMatrix();
}




void translate() {
	glPushMatrix();
	glLoadIdentity();
	/*******************************************************************/
	//(PA #3) :선택된 오브젝트의 평행이동을 구현하십시오.
	//1. modeling space 모드일 경우 드래그 거리에 따라 선택된 축에 대해 평행이동하게 구현하십시오.
	//2. viewing space 모드일 경우 x, y키가 입력되었다면 xy 평면에 대해서,
	// z키가 입력되었다면 카메라 시점과 평행하게 앞 뒤로 이동하게 구현하십시오.
	/*******************************************************************/

	//curobj true이면 소, 아니면 bunny

	if (keyv == true && translatekey == 1 && currentobj == true) {
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = cow2wld.m[i][j];
				cow2wld.m[i][j] = cam2wld[cameraIndex].m[i][j];
			}
		} //a 매트릭스에 cow2wld 복사함, 그리고 [0][0] ~[2][2] 까지의 자리에 카메라 좌표계를 설정해준다.

		glMultMatrixf(cow2wld.matrix()); //그리고 바꾼 matrix 로드
		glTranslatef(lenX / 10.0, lenY / 10.0, 0); // 그를 기준으로 translate
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				cow2wld.m[i][j] = a.m[i][j]; //cow2wld에 원래의 소 좌표계 복사
			}
		}
	}
	if (keyv == true && translatekey == 1 && currentobj == false) {
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = bunny2wld.m[i][j];
				bunny2wld.m[i][j] = cam2wld[cameraIndex].m[i][j];
			}
		}
		glMultMatrixf(bunny2wld.matrix());
		glTranslatef(lenX / 10.0, lenY / 10.0, 0);
		glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				bunny2wld.m[i][j] = a.m[i][j];
			}
		}
	}

	// v모드에서 translate xy평면 기준


	if (keyv == true && translatekey == 2 && currentobj == true) {
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = cow2wld.m[i][j];
				cow2wld.m[i][j] = cam2wld[cameraIndex].m[i][j];
			}
		}
		glMultMatrixf(cow2wld.matrix());
		glTranslatef(0.0, 0.0, lenX / 10.0); //위랑 완전히 동일하지만, translate할 때 z축 기준이므로 z으로 translate
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				cow2wld.m[i][j] = a.m[i][j];
			}
		}
	}

	if (keyv == true && translatekey == 2 && currentobj == false) {
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = bunny2wld.m[i][j];
				bunny2wld.m[i][j] = cam2wld[cameraIndex].m[i][j];
			}
		}
		glMultMatrixf(bunny2wld.matrix());
		glTranslatef(0.0, 0.0, lenX / 10.0);
		glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				bunny2wld.m[i][j] = a.m[i][j];
			}
		}
	}

	// v모드에서 translate z축 기준

	if (keym == true && currentobj == true) {
		glMultMatrixf(cow2wld.matrix());
		len = len / 10; // len으로만 하면 너무 많이 움직여서 10을 나눠줌
		if (dragon == 1)
			glTranslatef(len, 0, 0);
		else if (dragon == 2)
			glTranslatef(0, len, 0);
		else if (dragon == 3)
			glTranslatef(0, 0, len);
		//x y z 각각에 맞는 평행이동을 시켜주었다.
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix()); //변형된 modelview matrix를 소에 저장

	}

	if (keym == true && currentobj == false) {
		glMultMatrixf(bunny2wld.matrix());
		len = len / 10; // len으로만 하면 너무 많이 움직여서 10을 나눠줌
		if (dragon == 1)
			glTranslatef(len, 0, 0);
		else if (dragon == 2)
			glTranslatef(0, len, 0);
		else if (dragon == 3)
			glTranslatef(0, 0, len);
		//x y z 각각에 맞는 평행이동을 시켜주었다.
		glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix()); //변형된 modelview matrix를 소에 저장

	}

	//m모드에서 마우스에 맞게 obj 움직임


	glPopMatrix();
}




void idle() {

	/*******************************************************************/
	//(PA #3) : 추가적인 입력이 없을 때의 움직임을 구현하십시오.
	/*******************************************************************/
	if ((keym == true && keyr == true && keystate != 0))
		rotate();
	glutPostRedisplay();
}





//------------------------------------------------------------------------------
void main(int argc, char* argv[])
{
	width = 800;
	height = 600;
	frame = 0;
	glutInit(&argc, argv);							// Initialize openGL.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// Initialize display mode. This project will use float buffer and RGB color.
	glutInitWindowSize(width, height);				// Initialize window size.
	glutInitWindowPosition(100, 100);				// Initialize window coordinate.
	glutCreateWindow("PA4");

	glutDisplayFunc(display);						// Register display function to call that when drawing screen event is needed.
	glutReshapeFunc(reshape);						// Register reshape function to call that when size of the window is changed.
	glutKeyboardFunc(onKeyPress);					// Register onKeyPress function to call that when user presses the keyboard.
	glutMouseFunc(onMouseButton);					// Register onMouseButton function to call that when user moves mouse.
	glutMotionFunc(onMouseDrag);					// Register onMouseDrag function to call that when user drags mouse.

	glutIdleFunc(idle);

	glutSpecialFunc(SpecialKey);

	initialize();									// Initialize the other thing.
	glutMainLoop();									// Execute the loop which handles events.
}