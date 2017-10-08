#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>

//==============================================================================
class Complex
	//==============================================================================
{
public:
	float re, im;

	Complex(float re = 0, float im = 0) : re(re), im(im) {}
};

Complex operator+(const Complex& c1, const Complex& c2)
{
	return Complex(c1.re + c2.re, c1.im + c2.im);
}

Complex operator*(const Complex& c1, const Complex& c2)
{
	return Complex(c1.re*c2.re - c1.im*c2.im, c1.re*c2.im + c1.im*c2.re);
}


//==============================================================================

// glut callbacks
void display();
void keyboard(unsigned char k, int x, int y);
void mouse(int button, int state, int x, int y);
void reshape(int w, int h);

float left = -1.0;
float right = 1.0;
float bottom = -1.0;
float top = 1.0;

Complex c(0.109, 0.603);
int width = 512, height = 512;
bool doJuliaSet = true;

//------------------------------------------------------------------------------
void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("PA1");

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutReshapeFunc(reshape);

	glutMainLoop();
}

//------------------------------------------------------------------------------
void julia(Complex p, Complex c, int& its, float& R)
{
	float rSqr;
	int maxIterations = 256;
	for (its = 0; its < maxIterations; its++)
	{
		p = p*p + c;
		rSqr = p.re*p.re + p.im*p.im;
		if (rSqr > 4)
			break;
	}
	R = sqrt(rSqr);
}

//------------------------------------------------------------------------------
void mandelbrot(Complex c, int& its, float& R)
{
	float rSqr;
	int maxIterations = 256;

	Complex p(0, 0);

	for (its = 0; its < maxIterations; its++)
	{
		p = p*p + c;
		rSqr = p.re*p.re + p.im*p.im;
		if (rSqr > 4)
			break;
	}
	R = sqrt(rSqr);
}

//------------------------------------------------------------------------------
void display()
{
	// Clear the screen
	glClearColor(0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Setup the viewing matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, right, bottom, top);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/*******************************************************************/
	//(PA #1) : 코드 수정이 필요한 부분입니다.
	//수정한 내용을 주석으로 작성하여 제출하십시오.
	/*******************************************************************/
	float delta = (right - left) / float(width);
	float delta2 = (top - bottom) / float(height); //height를 변수로 하는 delta2를 만든다. 자세한 설명은 밑에 달겠습니다.
	printf("%lf %lf %d %d\n", delta, delta2, width, height);
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			// convert pixel location to world coordinates
			float x = left + i*delta;
			float y = bottom + j*delta2;

			// test for convergence
			int its;
			float R;
			Complex p(x, y);

			if (doJuliaSet)
				julia(p, c, its, R);
			else
				mandelbrot(p, its, R);

			// turn iterations and radius to color 
			if (its == 256)
				glColor3d(0, 0, 0);
			else
			{
				float r = R / float(3);
				float g = its / float(128);
				float b = R / float(its + 1);
				glColor3d(r, g, b);
			}

			// draw polygon			
			glBegin(GL_POLYGON);
			glVertex2d(x, y);
			glVertex2d(x, y + delta2);
			glVertex2d(x + delta, y + delta2);
			//사각형형태로 화면에 맞게 도트를 찍어주면서 이미지를 출력하는 건데,
			//이를 원래 코드와 같이 y에도 delta를 더해줄 시에는 width만이 기준이 되므로 도트를 제대로 찍지 못한다.
			//그러므로 height를 기준으로 하는 delta2를 만들어서 비율에 맞게 도트를 출력하면 올바르게 출력된다.
			glVertex2d(x + delta, y);
			glEnd();
		}
		/******************************************************************/
		glFlush();
	}
}


//-----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	if ((key == 'r') || (key == 'R')) //reset
	{
		// return to initial position
		c = Complex(0.109, 0.603);
		left = -1;
		right = 1;
		bottom = -1;
		top = 1;

		display();
	}
	else if (key == ' ') //switch
	{
		doJuliaSet = !doJuliaSet;
		display();
	}
}

//-----------------------------------------------------------------------------
void mouse(int button, int state, int mx, int my)
{
	// Convert Window Coordinates to World Coordinates
	float x = ((right - left) * mx / float(width)) + left;
	float y = ((top - bottom) * (1 - my / float(height))) + bottom;

	float dx = (right - left);
	float dy = (top - bottom);

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
	{
		left = x - dx / 4;
		right = x + dx / 4;
		bottom = y - dy / 4;
		top = y + dy / 4;
		display();
	}
}

//------------------------------------------------------------------------------
void reshape(int w, int h)
{

	width = w;
	height = h;
	glViewport(0, 0, w, h);

}