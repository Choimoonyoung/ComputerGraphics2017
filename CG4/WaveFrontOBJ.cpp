#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <GL/glut.h>
using namespace std;

#include "WaveFrontOBJ.h" 

//------------------------------------------------------------------------------
// 객체를 생성하면서 filename 에서 object 를 읽는다
// Construct object and read object from filename
WaveFrontOBJ::WaveFrontOBJ(char *filename) 
{
	isFlat = true;
	mode = GL_POLYGON;

	char *line = new char[200];
	char *line_back = new char[200];
	char wspace[] = " \t\n";
	char separator[] = "/";
	char *token;
	int indices[3];
	float x, y, z;
	float tex_u, tex_v;

	ifstream file(filename);
	if ( !file ) {
		cerr <<"Cannot open file: " <<filename <<" exiting." <<endl;
		exit ( -1 );
	}

	while ( !file.eof() ) {
		file.getline( line, 199 );
		// first, strip off comments
		if ( line[0] == '#' )
			continue;
		else if ( !strcmp( line, "" ) )
			continue;
		else {
			strcpy( line_back, line ); // strtok destroys line.
      
			token = strtok( line, wspace);

			if ( !strcmp( token, "v" ) ) {
				x = atof( strtok( NULL, wspace ) );
				y = atof( strtok( NULL, wspace ) );
				z = atof( strtok( NULL, wspace ) );
				verts.push_back( Vertex( x, y, z ) );
			}

			else if ( !strcmp( token, "vn" ) ) {
				x = atof( strtok( NULL, wspace ) );
				y = atof( strtok( NULL, wspace ) );
				z = atof( strtok( NULL, wspace ) );
				Vector vn(x, y, z);
				vn.Normalize();
				normals.push_back( vn );
			}

			else if ( !strcmp( token, "vt" ) ) {
				tex_u = atof( strtok( NULL, wspace ) );
				tex_v = atof( strtok( NULL, wspace ) );
				texCoords.push_back( TexCoord(tex_u, tex_v ) );
			}

			else if ( !strcmp( token, "f" ) ) {
				int vi = (int)vIndex.size();
				faces.push_back( Face( vi ) );
				Face& curFace = faces.back();
				for (char *p = strtok( NULL, wspace ); p ; p = strtok( NULL, wspace ) ) {
					indices[0] = 0; 
					indices[1] = 0;
					indices[2] = 0;
					char* pos = p;
					int len = (int)strlen(p);

					for ( int j=0, i=0;  j <= len && i < 3; j++ ) {
						if ( p[j] == '/' || p[j] == 0) {
							p[j] = 0;
							indices[i++] = atoi( pos );
							pos = p + j+1;
						}
					}

					vIndex.push_back( indices[0] - 1 );
					tIndex.push_back( indices[1] - 1 );
					nIndex.push_back( indices[2] - 1 );                        
					curFace.vCount++;

					if (indices[2] != 0)
						isFlat = false;
				}
				if( curFace.vCount > 2 ){
					curFace.normal = faceNormal(verts[vIndex[vi]], verts[vIndex[vi+1]], verts[vIndex[vi+2]] );
					curFace.normal.Normalize();
					faceNormals.push_back(curFace.normal);
				}
			}


			else if ( !strcmp( token, "g" ) ) {      // group
			}
			else if ( !strcmp( token, "s" ) ) {      // smoothing group
			}
			else if ( !strcmp( token, "u" ) ) {      // material line
			}
			else if ( !strcmp( token, "" ) ) {       // blank line
			}
			else {
				cout <<line_back <<endl;
			}
		}
	}
	
	vertexNormal();

	computeBoundingBox();
}


//------------------------------------------------------------------------------
Vector WaveFrontOBJ::faceNormal(Vertex& v0, Vertex& v1, Vertex& v2) {
	/*******************************************************************/
	//(PA #4) : 세 점의 좌표를 이용하여 face normal을 계산하는 함수를 완성하십시오.
	// - 계산한 face normal이 각 Face class의 normal 에 저장 되도록 구현하십시오.
	/*******************************************************************/
	Vector a = v0.pos - v1.pos;
	Vector b = v2.pos - v1.pos;
	b = b.Vector::CrossProduct(a);

	return b;
}

void WaveFrontOBJ::vertexNormal() {
	/*******************************************************************/
	//(PA #4) : 주변 face normal을 이용하여 vertex normal을 계산하는 함수를 완성하십시오.
	// - 계산한 vertex normal이 각 Vertex Class의 normal에 저장 되도록 구현하십시오.
	/*******************************************************************/

	for (int i = 0; i < (int)verts.size(); i++) {
		Vertex& curvtx = verts[i];
		for (int j = 0; j < (int)faces.size(); j++) {
			Face curface = faces[j];
			for (int k = curface.vIndexStart; k < curface.vIndexStart + curface.vCount; k++) {
				if (i == vIndex[k]) {
					curvtx.normal = curvtx.normal + curface.normal;
					curvtx.count++;
					break;
				}
			}
		}
		curvtx.normal.x /= (float)curvtx.count;
		curvtx.normal.y /= (float)curvtx.count;
		curvtx.normal.z /= (float)curvtx.count;
		curvtx.normal.Vector::Normalize();
	}
}

//------------------------------------------------------------------------------
// OpenGL API 를 사용해서 파일에서 읽어둔 object 를 그리는 함수.
// Draw object which is read from file
void WaveFrontOBJ::Draw() {
	int i;

	for (int f = 0; f < (int)faces.size(); f++) {
		Face& curFace = faces[f];        
		glBegin(mode);
		for (int v = 0; v < curFace.vCount; v++) {
			int vi = curFace.vIndexStart + v;
			
			if (isFlat) {
				if (v == 0) {
					glNormal3f(curFace.normal.x, curFace.normal.y, curFace.normal.z);
				}
			}
			
			else if ((i = vIndex[vi]) >= 0) {
				glNormal3f(verts[i].normal.x, verts[i].normal.y, verts[i].normal.z);
			}
			
			if ((i = tIndex[vi]) >= 0) {
				glTexCoord2f(texCoords[i].u, texCoords[i].v);
			}
			if ((i = vIndex[vi]) >= 0) {
				glVertex3f(verts[i].pos.x, verts[i].pos.y, verts[i].pos.z);
			}
		}
		glEnd();
	}
}

void WaveFrontOBJ::Draw_FN() {
	glDisable(GL_LIGHTING);
	/*******************************************************************/
	//(PA #4) : 각 face에 대해 face normal을 그리는 함수를 작성하십시오.
	// - 작성한 함수는 drawCow, drawbunny에 활용하십시오.
	/*******************************************************************/
	for (int i = 0; i < (int)faces.size(); i++) {
		Face curface = faces[i];
		Vector a(0, 0, 0);
		for (int j = curface.vIndexStart; j < curface.vCount + curface.vIndexStart; j++) 
			a = a + verts[vIndex[j]].pos;
		a.x /= (float)curface.vCount;
		a.y /= (float)curface.vCount;
		a.z /= (float)curface.vCount;

		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_LINES);
		glVertex3f(a.x, a.y, a.z);
		a = a + curface.normal;
		glVertex3f(a.x, a.y, a.z);
		glEnd();
	}
	glEnable(GL_LIGHTING);
}

void WaveFrontOBJ::Draw_VN() {
	glDisable(GL_LIGHTING);
	/*******************************************************************/
	//(PA #4) : 각 vertex에 대해 vertex normal을 그리는 함수를 작성하십시오.
	// - 작성한 함수는 drawCow, drawbunny에 활용하십시오.
	/*******************************************************************/
	for (int i = 0; i < (int)verts.size(); i++) {
		Vertex curvtx = verts[i];
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex3f(curvtx.pos.x, curvtx.pos.y, curvtx.pos.z);
		glVertex3f(curvtx.pos.x + curvtx.normal.x, curvtx.pos.y + curvtx.normal.y, curvtx.pos.z + curvtx.normal.z);
		glEnd();
	}
	glEnable(GL_LIGHTING);
}

//------------------------------------------------------------------------------
void WaveFrontOBJ::computeBoundingBox()
{
	if( verts.size() > 0 )
	{
		bbmin.pos.x = verts[0].pos.x;
		bbmin.pos.y = verts[0].pos.y;
		bbmin.pos.z = verts[0].pos.z;
		bbmax.pos.x = verts[0].pos.x;
		bbmax.pos.y = verts[0].pos.y;
		bbmax.pos.z = verts[0].pos.z;
		for( int i=1; i < (int)verts.size(); i++ )
		{
			if( verts[i].pos.x < bbmin.pos.x ) bbmin.pos.x = verts[i].pos.x;
			if( verts[i].pos.y < bbmin.pos.y ) bbmin.pos.y = verts[i].pos.y;
			if( verts[i].pos.z < bbmin.pos.z ) bbmin.pos.z = verts[i].pos.z;
			if( verts[i].pos.x > bbmax.pos.x ) bbmax.pos.x = verts[i].pos.x;
			if( verts[i].pos.y > bbmax.pos.y ) bbmax.pos.y = verts[i].pos.y;
			if( verts[i].pos.z > bbmax.pos.z ) bbmax.pos.z = verts[i].pos.z;
		}
	}
}