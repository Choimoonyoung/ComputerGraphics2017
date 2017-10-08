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

int cameraIndex, camID;
vector<Matrix> wld2cam, cam2wld;

// Variables for 'cow' object.
Matrix cow2wld;
WaveFrontOBJ* cow;
int cowID;

// Variables for 'bunny' object.
Matrix bunny2wld;
WaveFrontOBJ* bunny;
int bunnyID;

unsigned floorTexID;

int frame = 0;
int width, height;
int oldX, oldY;

void translate();
void rotate();

int selectMode;
bool picking = false;

/*******************************************************************/
//(PA #3) : ������ �ذ��ϱ� ���� ������ �߰������� �����Ͻʽÿ�.
/*******************************************************************/

bool keym = false;
bool keyv = false;
int keystate = 0;  // x?y?z������ �����ϱ� ���ؼ� ����
GLfloat theta = 0.5; // ���� ũ��
bool keyr = false;
int len = 0; //��ŭ translate���� �����ϴ� ����
int dragon = 0; //drag�� on �Ǿ��ִٴ� �� (m��忡��)
int translatekey = 0; // xy���? z���?
int lenX = 0; //v��忡�� ��ŭ translate���� �����ϴ� ����
int lenY = 0; //v��忡�� ��ŭ translate���� �����ϴ� ���� (y����)
bool currentobj = true; //true�� ��, false�� bunny
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
	cow = new WaveFrontOBJ("cow.obj");
	
	cowID = glGenLists(1);									// Create display lists
	glNewList(cowID, GL_COMPILE);							// Begin compiling the display list using cowID
	cow->Draw();											// Draw the cow on display list.
	glEndList();											// Terminate compiling the display list. Now, you can draw cow using 'cowID'.

	// Make display list. After this, you can draw cow using 'cowID'.
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
	glCallList(cowID);										// Draw cow. 

	glPopMatrix();											// Pop the matrix in stack to GL. Change it the matrix before drawing cow.
}





void InitBunny(){
	bunny = new WaveFrontOBJ("bunny.obj");

	bunnyID = glGenLists(1);									// Create display lists
	glNewList(bunnyID, GL_COMPILE);							// Begin compiling the display list using bunnyID
	bunny->Draw();											// Draw the bunny on display list.
	glEndList();											// Terminate compiling the display list. Now, you can draw beethovna using 'bunnyID'.

	// Make display list. After this, you can draw bunny using 'bunnyID'.
	glPushMatrix();											// Push the current matrix of GL into stack.
	glLoadIdentity();										// Set the GL matrix Identity matrix.
	glTranslatef(0, -bunny->bbmin.pos.y, 8);					// Set the location of bunny.
	glRotatef(180, 0, 1, 0);								// Set the direction of bunny. These information are stored in the matrix of GL.
	glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());		// Read the modelview matrix about location and direction set above, and store it in bunny2wld matrix.
	glPopMatrix();											// Pop the matrix on stack to GL.

}





void drawBunny()
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
	glCallList(bunnyID);									// Draw bunny. 

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

	drawCamera();													// and draw all of them.
	drawFloor();													// Draw floor.

	drawCow();														// Draw cow.
	drawBunny();

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
	
	glShadeModel(GL_SMOOTH);			 // Set Flat Shading
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
	InitBunny();
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
				//(PA #3) Ŭ���� ��ǥ�� �ȼ� ���� ���� �ٸ� ������Ʈ�� �����ϰ� ����.
				/*******************************************************************/
				if (pixel_value == 32)
					currentobj = true; //32�� ��
				else if (pixel_value == 33)
					currentobj = false; //33�̸� bunny
				else
					printf("error\n"); //���� err
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
	//(PA #3) : ���콺�� �巡�� �Ÿ��� ���⿡ ���� ������Ʈ�� �����̵� �Ÿ� / ȸ�� ������ �����Ͻʽÿ�.
	//1. �Ϲ����� �����̵��� ��� ��ũ������ x���� ���� �巡���ϸ� �����̵��ϰ� �Ͻʽÿ�.
	//2. xy�� ��鿡 ���� �̵��ϴ� ��� x��, y�� �̵��� ���� ����Ͻʽÿ�.
	//3. viewing space�� ȸ���̵� ������ ��� 1.�� ���� ������� �����Ͻʽÿ�. 
	/*******************************************************************/
	if (keyv == true && translatekey == 1) { // xy���
		lenX = x - oldX;
		lenY = y - oldY;
		translate();
	} //v��忡��
	if (keyv == true && translatekey == 2) { //z��
		lenX = x - oldX;
		translate();
	}//v���

	if (keyv == true && keyr == true) { //viewing ��ǥ�� x�� ���� ȸ��
		lenY = y - oldY;
		rotate();
	} // v���

	if (keym == true) { 
		if ((abs(x - oldX) < 10.0) /*&& (abs((double)y - oldY - (x - oldX)) > 10.0)*/) {
			//y�����̵�(x�� ���� ����������), dragon�� 2�� �����ϰ� �ٲ� len�� �������� translate�Ѵ�.
			dragon = 2;
			len = y - oldY;
			printf("%d", len);
			translate();
		}
		else if ((x - oldX <= 0 && y - oldY >= 0) || (x - oldX >= 0 && y - oldY <= 0)) {
			//z �����̵�(x�� y�� �巡�׵� ������ ���� �ݴ���), dragon�� 3�� �����ϰ� �ٲ� len�� �������� translate�Ѵ�.
			dragon = 3;
			len = x - oldX;
			translate();
		}
		else if ((x - oldX >= 0 && y - oldY >= 0) || (x - oldX <= 0 && y - oldY <= 0)) {
			//x �����̵�(x�� y�� �巡�� �� ������ �Ȱ��� x�� ���� �������ٸ�), dragon�� 1�� �����ϰ� �ٲ� len�� �������� translate�Ѵ�.
			dragon = 1;
			len = x - oldX;
			translate();
		}
		else
			dragon = 0;
	} //m���
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
	//ī�޶� ���� ����
	if ((key >= '1') && (key <= '5'))
		cameraIndex = key - '1';

	//���� �ش��ϴ� key�� ������ �� ������ �ٲ���

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
	if (key == 'v') { //v����϶��� m ����, m����϶��� ������ v����.
		if (keyv == false){
			keyv = true;
			keym = false;
		}
		else {
			translatekey = 0;
			keyv = false;
		}
	}

	if (key == 'x') { //keystate�� m���, translatekey�� v���
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
	//(PA #3) : �������� �����ϴ� ��� Ű���� �Է¿� ���� �ڵ带 �߰��� �ۼ��Ͻʽÿ�.
	/*******************************************************************/

	glutPostRedisplay();
}





void rotate(){
	glPushMatrix();
	glLoadIdentity();
	//curobj�� true�̸� ��, false�̸� bunny�� ���õ� ���ǹ���
	if (keym == true&&keyr == true&&currentobj == true) {
		glMultMatrixf(cow2wld.matrix());
		if (keystate == 1) 
			glRotatef(theta, 1, 0, 0);
		else if (keystate == 2)
			glRotatef(theta, 0, 1, 0);
		else if (keystate == 3)
			glRotatef(theta, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
	} 
	if (keym == true && keyr == true && currentobj == false ) {
		glMultMatrixf(bunny2wld.matrix());
		if (keystate == 1)
			glRotatef(theta, 1, 0, 0);
		else if (keystate == 2)
			glRotatef(theta, 0, 1, 0);
		else if (keystate == 3)
			glRotatef(theta, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix());
	}
	
	//m��忡�� �ڱ� ��ǥ�� ���� ȸ��


	if (keyv == true&& keyr == true&&currentobj == true) {
		glMultMatrixf(cow2wld.matrix()); //matrix�� �ε� �մϴ�.
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = cow2wld.m[i][j];
			}
		} //a matrix�� cow2wld matrix�� [0][0]~~[2][2]���� �����մϴ�.
		Matrix b;
		a.m[3][3] = 1;
		a.set(a.inverse().m);//a�� inverse �� b (viewing space�� x�� ��ǥ)�� �����ݴϴ�. �׸��� ���� [0][0],[1][0],[2][0] �� �������� rotate�� �ݴϴ�.
		b.m[0][0] = cam2wld[cameraIndex].m[0][0];
		b.m[1][0] = cam2wld[cameraIndex].m[0][1];
		b.m[2][0] = cam2wld[cameraIndex].m[0][2];
		a.set(a.multiply(b).m);
		glRotatef(lenY, a.m[0][0], a.m[1][0], a.m[2][0]);
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
	}

	if (keyv == true && keyr == true&&currentobj == false) {
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

	//v��忡�� viewing ��ǥ���� x�� ���� ȸ��


	/*******************************************************************/
	//(PA #3) : ���õ� ������Ʈ�� ȸ���� �����Ͻʽÿ�.
	//1. modeling space ����� ��� 'r'Ű�� ȸ�����°� ON �Ǿ��� ��� ���õ� �࿡ ���� �ڵ����� ȸ���ǰ� �Ͻʽÿ�.
	//2. viewing space ����� ��� x���� �������� �巡�� �Ÿ��� ���� ȸ���ϰ� �Ͻʽÿ�.
	/*******************************************************************/
	glPopMatrix();
}





void translate(){
	glPushMatrix();
	glLoadIdentity();
	/*******************************************************************/
	//(PA #3) :���õ� ������Ʈ�� �����̵��� �����Ͻʽÿ�.
	//1. modeling space ����� ��� �巡�� �Ÿ��� ���� ���õ� �࿡ ���� �����̵��ϰ� �����Ͻʽÿ�.
	//2. viewing space ����� ��� x, yŰ�� �ԷµǾ��ٸ� xy ��鿡 ���ؼ�,
	// zŰ�� �ԷµǾ��ٸ� ī�޶� ������ �����ϰ� �� �ڷ� �̵��ϰ� �����Ͻʽÿ�.
	/*******************************************************************/
	
	//curobj true�̸� ��, �ƴϸ� bunny
	
	if (keyv == true && translatekey == 1&&currentobj == true) {
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = cow2wld.m[i][j];
				cow2wld.m[i][j] = cam2wld[cameraIndex].m[i][j];
			}
		} //a ��Ʈ������ cow2wld ������, �׸��� [0][0] ~[2][2] ������ �ڸ��� ī�޶� ��ǥ�踦 �������ش�.

		glMultMatrixf(cow2wld.matrix()); //�׸��� �ٲ� matrix �ε�
		glTranslatef(lenX/10.0, lenY/10.0, 0); // �׸� �������� translate
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix());
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				cow2wld.m[i][j] = a.m[i][j]; //cow2wld�� ������ �� ��ǥ�� ����
			}
		}
	}
	if (keyv == true && translatekey == 1&&currentobj == false) {
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

	// v��忡�� translate xy��� ����


	if (keyv == true && translatekey == 2&&currentobj == true) {
		Matrix a;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				a.m[i][j] = cow2wld.m[i][j];
				cow2wld.m[i][j] = cam2wld[cameraIndex].m[i][j];
			}
		}
		glMultMatrixf(cow2wld.matrix());
		glTranslatef(0.0, 0.0, lenX / 10.0); //���� ������ ����������, translate�� �� z�� �����̹Ƿ� z���� translate
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

	// v��忡�� translate z�� ����

	if (keym == true&&currentobj == true) {
		glMultMatrixf(cow2wld.matrix());
		len = len / 10; // len���θ� �ϸ� �ʹ� ���� �������� 10�� ������
		if (dragon == 1)
			glTranslatef(len, 0, 0);
		else if (dragon == 2)
			glTranslatef(0, len, 0);
		else if (dragon == 3)
			glTranslatef(0, 0, len);
		//x y z ������ �´� �����̵��� �����־���.
		glGetFloatv(GL_MODELVIEW_MATRIX, cow2wld.matrix()); //������ modelview matrix�� �ҿ� ����

	}
	
	if (keym == true&& currentobj == false) {
		glMultMatrixf(bunny2wld.matrix());
		len = len / 10; // len���θ� �ϸ� �ʹ� ���� �������� 10�� ������
		if (dragon == 1)
			glTranslatef(len, 0, 0);
		else if (dragon == 2)
			glTranslatef(0, len, 0);
		else if (dragon == 3)
			glTranslatef(0, 0, len);
		//x y z ������ �´� �����̵��� �����־���.
		glGetFloatv(GL_MODELVIEW_MATRIX, bunny2wld.matrix()); //������ modelview matrix�� �ҿ� ����

	}

	//m��忡�� ���콺�� �°� obj ������


	glPopMatrix();
}





void idle() {

	/*******************************************************************/
	//(PA #3) : �߰����� �Է��� ���� ���� �������� �����Ͻʽÿ�.
	/*******************************************************************/
	if ((keym == true && keyr == true&& keystate != 0))
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
	glutCreateWindow("PA3");

	glutDisplayFunc(display);						// Register display function to call that when drawing screen event is needed.
	glutReshapeFunc(reshape);						// Register reshape function to call that when size of the window is changed.
	glutKeyboardFunc(onKeyPress);					// Register onKeyPress function to call that when user presses the keyboard.
	glutMouseFunc(onMouseButton);					// Register onMouseButton function to call that when user moves mouse.
	glutMotionFunc(onMouseDrag);					// Register onMouseDrag function to call that when user drags mouse.

	glutIdleFunc(idle);
	
	initialize();									// Initialize the other thing.
	glutMainLoop();									// Execute the loop which handles events.
}