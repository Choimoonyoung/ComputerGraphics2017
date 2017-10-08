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

unsigned floorTexID;

int frame = 0;
int width, height;
int oldX, oldY;

void cow_translate();
void cow_rotate();

/*******************************************************************/
//(PA #2) : 과제를 해결하기 위한 변수를 추가적으로 선언하십시오.
/*******************************************************************/
float theta = 5.0; //회전 각도
char keystate=0; //x y z축중 어느축을 기준으로 회전할 지 결정하는 변수
int ron = 0;//r이 눌려있는가? 1이면 눌려있는 것
int state = 0; //
int len = 0; //어느 정도 평행이동 할 것인지 결정
int dragon = 0;//드래그가 켜져있는가?
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





void InitCamera() {

	// initialize camera frame transforms.
	for (int i = 0; i < cameraCount; i++)
	{
		float* c = cameras[i];											// 'c' points the coordinate of i-th camera.
		wld2cam.push_back(Matrix());								// Insert {0} matrix to wld2cam vector.
		glPushMatrix();													// Push the current matrix of GL into stack.

		glLoadIdentity();												// Set the GL matrix Identity matrix.
		gluLookAt(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8]);		// Setting the coordinate of camera.
		glGetFloatv(GL_MODELVIEW_MATRIX, wld2cam[i].matrix());			// Read the world-to-camera matrix computed by gluLookAt.
		cam2wld.push_back(wld2cam[i].inverse());						// Get the camera-to-world matrix.

		glPopMatrix();													// Transfer the matrix that was pushed the stack to GL.

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

			glutSolidSphere(1.0, 10, 10);
			drawAxis(5);													// Draw x, y, and z axis.

			glPopMatrix();													// Call the matrix on stack. wld2cam[cameraIndex].matrix() in here.
		}
	}
}





/*********************************************************************************
* Draw 'cow' object.
**********************************************************************************/
void InitCow() {
	cow = new WaveFrontOBJ("cow.obj");

	// Make display list. After this, you can draw cow using 'cowID'.
	glPushMatrix();						// Push the current matrix of GL into stack.
	glLoadIdentity();					// Set the GL matrix Identity matrix.
	glTranslatef(0, -cow->bbmin.pos.y, -8);	// Set the location of cow.
	glRotatef(-90, 0, 1, 0);			// Set the direction of cow. These information are stored in the matrix of GL.
	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());	// Read the modelview matrix about location and direction set above, and store it in cow2wld matrix.
	glPopMatrix();						// Pop the matrix on stack to GL.

}



void drawCow()
{
	glPushMatrix();		// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing cow.

						// The information about location of cow to be drawn is stored in cow2wld matrix.
						// If you change the value of the cow2wld matrix or the current matrix, cow would rotate or move.
	glMultMatrixf(cow2wld.matrix());

	cow->Draw();						// Draw the cow on display list.
	drawAxis(5);						// Draw x, y, and z axis.

	glPopMatrix();			// Pop the matrix in stack to GL. Change it the matrix before drawing cow.
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

	glColor3d(0.35, 0.2, 0.1);

	// Draw background rectangle.
	glBegin(GL_POLYGON);
	glVertex3f(2000, -0.2, 2000);
	glVertex3f(2000, -0.2, -2000);
	glVertex3f(-2000, -0.2, -2000);
	glVertex3f(-2000, -0.2, 2000);
	glEnd();


	// Set color of the floor.
	// Assign checker-patterned texture.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, floorTexID);

	// Draw the floor. Match the texture's coordinates and the floor's coordinates resp. 
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	glVertex3d(-12, -0.1, -12);		// Texture's (0,0) is bound to (-12,-0.1,-12).
	glTexCoord2d(1, 0);
	glVertex3d(12, -0.1, -12);		// Texture's (1,0) is bound to (12,-0.1,-12).
	glTexCoord2d(1, 1);
	glVertex3d(12, -0.1, 12);		// Texture's (1,1) is bound to (12,-0.1,12).
	glTexCoord2d(0, 1);
	glVertex3d(-12, -0.1, 12);		// Texture's (0,1) is bound to (-12,-0.1,12).
	glEnd();

	glDisable(GL_TEXTURE_2D);

	drawAxis(5);				// Draw x, y, and z axis.
}





/*********************************************************************************
* Call this part whenever display events are needed.
* Display events are called in case of re-rendering by OS. ex) screen movement, screen maximization, etc.
* Or, user can occur the events by using glutPostRedisplay() function directly.
* this part is called in main() function by registering on glutDisplayFunc(display).
**********************************************************************************/
void display()
{
	glClearColor(0, 0.6, 0.8, 1);									// Clear color setting

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear the screen

	drawCamera();													// and draw all of them.
	drawFloor();													// Draw floor.

	drawCow();														// Draw cow.

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
	glShadeModel(GL_FLAT);			 // Set Flat Shading
	glEnable(GL_DEPTH_TEST);         // Enables Depth Testing

									 // Define lighting for the scene
	glEnable(GL_LIGHTING);
	float light_pos[] = { 1.0, 1.0, 1.0, 0.0 };
	float light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	float light_diffuse[] = { 0.9, 0.9, 0.9, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);

	InitCamera();
	InitCow();
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
	//(PA #2) :추가적인 코드 작성이 필요합니다.
	//마우스의 이동 거리를 통해 오브젝트의 평행이동 거리를 결정하십시오.
	/*******************************************************************/
	if ((abs(x - oldX)<10.0) /*&& (abs((double)y - oldY - (x - oldX)) > 10.0)*/) {
		//y평행이동(x가 조금 움직였으면), dragon을 2로 설정하고 바꾼 len을 기준으로 translate한다.
		dragon = 2;
		len = y - oldY;
		printf("%d", len);
		cow_translate();
	}
	else if ((x - oldX <= 0 && y - oldY >= 0) || (x - oldX >= 0 && y - oldY <= 0)) {
		//z 평행이동(x와 y가 드래그된 방향이 서로 반대라면), dragon을 3로 설정하고 바꾼 len을 기준으로 translate한다.
		dragon = 3;
		len = x - oldX;
		cow_translate();
	}
	else if ((x - oldX >= 0 && y - oldY >= 0) || (x - oldX <= 0 && y - oldY <= 0)) {
		//x 평행이동(x와 y가 드래그 된 방향이 똑같고 x가 많이 움직였다면), dragon을 1로 설정하고 바꾼 len을 기준으로 translate한다.
		dragon = 1;
		len = x - oldX;
		cow_translate();
	}
	else
		dragon = 0;
	
	oldX = x;
	oldY = y;
	glutPostRedisplay();
}





/*********************************************************************************
* Call this part whenever user types keyboard.
* This part is called in main() function by registering on glutKeyboardFunc(onKeyPress).
**********************************************************************************/

void onKeyPress(unsigned char key, int x, int y)
{
	//카메라 시점 변경
	if ((key >= '1') && (key <= '5'))
		cameraIndex = key - '1';

	/*******************************************************************/
	//(PA #2) :추가적인 코드 작성이 필요합니다.
	//오브젝트의 회전 적용여부와 모델링 좌표계의 축을 결정하기 위한 함수를 작성하십시오.
	/*******************************************************************/

	//x y z를 눌렀을 때는  keystate를 변경한다.
	if (key == 'x')
		keystate = 1;
	else if (key == 'y')
		keystate = 2;
	else if (key == 'z')
		keystate = 3;
	else if (key == 'r' && keystate !=0) {
		//keystate가 변경된 후 r을 눌렀을 때는 r이 꺼져있으면 켜주고 켜져있으면 꺼주고 keystate를 초기화한다.
		if (ron == 0)
			ron = 1;
		else if (ron == 1) {
			ron = 0;
			keystate = 0;
		}
		if (ron ==1 && keystate != 0) {
			cow_rotate();
		} //둘다 켜져있을 경우 rotate
	}

	glutPostRedisplay();
}





void cow_rotate() {
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixf(cow2wld.matrix()); //load current transformation matrix of object

									 /*******************************************************************/
									 //(PA #2) :추가적인 코드 작성이 필요합니다.
									 //현재 선택된 축에 대하여 오브젝트의 회전을 구현하십시오.
									 /*******************************************************************/
	if (keystate == 1)
		glRotatef(theta, 1, 0, 0);
	else if (keystate == 2)
		glRotatef(theta, 0, 1, 0);
	else if (keystate == 3)
		glRotatef(theta, 0, 0, 1);
	//x y z 각각에 어울리는 회전이동을 해준다. glRotatef는 원래 소의 transformation matrix에 회전변환matrix를 곱해주는 것
	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
	glPopMatrix();
}





void cow_translate() {
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixf(cow2wld.matrix()); //load current transformation matrix of object

									 /*******************************************************************/
									 //(PA #2) :추가적인 코드 작성이 필요합니다.
									 //현재 선택된 축에 대하여 오브젝트의 평행이동을 구현하십시오.
									 /*******************************************************************/
	len = len / 10; // len으로만 하면 너무 많이 움직여서 10을 나눠줌
	if (dragon == 1)
		glTranslatef(len, 0, 0);
	else if (dragon == 2)
		glTranslatef(0, len, 0);
	else if (dragon == 3)
		glTranslatef(0, 0, len);
	//x y z 각각에 맞는 평행이동을 시켜주었다.
	glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix()); //변형된 modelview matrix를 소에 저장
	glPopMatrix();
}





void idle() {
	/*******************************************************************/
	//(PA #2) :추가적인 코드 작성이 필요합니다.
	//추가적인 입력이 없을 때의 움직임을 구현하십시오.
	/*******************************************************************/
	if (ron == 1)
		cow_rotate(); //r이 켜져있으면 계속 rotate한다
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
	glutCreateWindow("PA2");

	glutDisplayFunc(display);						// Register display function to call that when drawing screen event is needed.
	glutReshapeFunc(reshape);						// Register reshape function to call that when size of the window is changed.
	glutKeyboardFunc(onKeyPress);					// Register onKeyPress function to call that when user presses the keyboard.
	glutMouseFunc(onMouseButton);					// Register onMouseButton function to call that when user moves mouse.
	glutMotionFunc(onMouseDrag);					// Register onMouseDrag function to call that when user drags mouse.

	glutIdleFunc(idle);

	initialize();									// Initialize the other thing.
	glutMainLoop();									// Execute the loop which handles events.
}