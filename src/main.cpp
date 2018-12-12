/*
Assignment 4
Eric Roberts
*/

#define _USE_MATH_DEFINES

#include <vector>
#include <map>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <fstream>

#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/glut.h"

#include "vec3.hpp"
#include "matrix.hpp"

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512
#define FULL_SCALE 10.0

Matrix test;

typedef std::map<char, std::string> Grammar;

Grammar grammar;
std::string fractal = "X";
float girth = 0.5;

int last_mpos[2];
int cur_mpos[2];
bool arcball_on = false;
float camera_pos[3] = { 0.0, 0.0, 20.0 };

struct State {
	Vec3 position;
	float radius;
	Vec3 axis[3];
};

float fractal_scale = FULL_SCALE;
float angle_increment;
int fractal_depth = 0;

typedef std::vector<Vec3> VertexBuffer;
std::vector<VertexBuffer> cylinder_list;
VertexBuffer spine;

bool show_girth = false;
bool show_spine = true;

void assert(bool condition, const char* message) {
	if (!condition) {
		throw std::runtime_error(message);
	}
}

std::string expandFractal(std::string fractal, Grammar grammar) {
	std::string result;
	for(int i = 0; i < (int)fractal.size(); ++i) {
		char c = fractal[i];
		bool replaced = false;
		for(auto it = grammar.begin(); it != grammar.end(); it++) {
			if (c == it->first) {
				result += it->second;
				replaced = true;
				break;
			}
		}
		if (!replaced && c != ' ') {
			result += c;
		}
	}
	return result;
}

void generateRodVertices(Vec3 v1, Vec3 v2, float r1, float r2, int sections = 6) {
	Vec3 l = v2 - v1;

	Vec3 x = l + Vec3(1, 1, 1);
	x -= Vec3::project(x, l);
	x.normalize();

	Vec3 y = Vec3::crossProduct(x, l);
	y.normalize();

	cylinder_list.resize(cylinder_list.size() + 1);
	VertexBuffer& cur_buffer = cylinder_list.back();

	for(int i = 0; i < sections + 1; i++) {
		float angle = 2.0 * M_PI * (i % sections) / sections;
		Vec3 r = std::cos(angle) * x + std::sin(angle) * y;
		Vec3 p1 = v1 + r1 * r;
		Vec3 p2 = v2 + r2 * r;

		cur_buffer.push_back(p1);
		cur_buffer.push_back(p2);
	}
}

void generateFractalVertices(std::string fractal, State initial_state,
		float fractal_scale, float angle_increment) {
	std::vector<State> state_stack;
	state_stack.push_back(initial_state);

	cylinder_list.clear();
	spine.clear();

	for(int i = 0; i < (int)fractal.size(); ++i) {
		State& cur_state = state_stack.back();
		switch(fractal[i]) {
			case 'F':
			case 'X':
				{
					// draw forward
					Vec3 p1 = cur_state.position;

					cur_state.position += fractal_scale * cur_state.axis[1];

					Vec3 p2 = cur_state.position;
					float radius1 = cur_state.radius;
					cur_state.radius *= (10.0 - fractal_scale)/10.0;

					generateRodVertices(p1, p2, radius1, cur_state.radius);

					spine.push_back(p1);
					spine.push_back(p2);

					float roll = angle_increment * fractal_scale / 5.0;
					Matrix rotation = calcRotationMatrix(roll, cur_state.axis[1]);
					for(int i = 0; i < 3; i++) {
						cur_state.axis[i] = cur_state.axis[i] * rotation;
					}

				}
				break;

			case '+':
				{
					// rotate counter-clockwise
					Matrix rotation = calcRotationMatrix(angle_increment, cur_state.axis[2]);
					for(int i = 0; i < 3; i++) {
						cur_state.axis[i] = cur_state.axis[i] * rotation;
					}
					rotation = calcRotationMatrix(M_PI / 3.0, cur_state.axis[1]);
					for(int i = 0; i < 3; i++) {
						cur_state.axis[i] = cur_state.axis[i] * rotation;
					}
				}
				break;

			case '-':
				{
					// rotate clockwise
					Matrix rotation = calcRotationMatrix(-angle_increment, cur_state.axis[2]);
					for(int i = 0; i < 3; i++) {
						cur_state.axis[i] = cur_state.axis[i] * rotation;
					}
					rotation = calcRotationMatrix(-M_PI / 3.0, cur_state.axis[1]);
					for(int i = 0; i < 3; i++) {
						cur_state.axis[i] = cur_state.axis[i] * rotation;
					}
				}
				break;

			case '[':
				// push stack
				state_stack.push_back(cur_state);
				break;

			case ']':
				// pop stack
				assert(state_stack.size() > 1, "popping initial state");
				state_stack.pop_back();
				break;

			case ' ':
			case '\t':
			case '\n':
			case '\v':
				break;

			default:
				std::cout << "invalid token: " << fractal[i] << '\n';
				throw std::runtime_error("invalid token");
		}
	}
}

void keyboardInput(unsigned char key, int x, int y) {
	switch(key) {
		case 'g':
		case 'G':
			show_girth = !show_girth;
			break;
		case 's':
		case 'S':
			show_spine = !show_spine;
			break;
	}
}

void generateFractal() {
	fractal = "X";
	for(int i = 0; i < fractal_depth; i++) {
		fractal = expandFractal(fractal, grammar);
	}
	fractal_scale = FULL_SCALE / std::pow(2, fractal_depth);

	State initial_state;
	initial_state.position = Vec3(0, -7, 0);
	initial_state.radius = girth;
	for(int i = 0; i < 3; i++) {
		Vec3 v;
		v[i] = 1;
		initial_state.axis[i] = v;
	}

	generateFractalVertices(fractal, initial_state, fractal_scale, angle_increment);
}

void specialKeyboardInput(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
			fractal_depth = std::min(7, fractal_depth + 1);
			generateFractal();
			break;
			
		case GLUT_KEY_DOWN:
			fractal_depth = std::max(0, fractal_depth - 1);
			generateFractal();
			break;
	}
}

void mouseAction(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				arcball_on = true;
				last_mpos[0] = cur_mpos[0] = x;
				last_mpos[1] = cur_mpos[1] = y;
			} else {
				arcball_on = false;
			}
			break;
			
		case 3: // mouse wheel up
			camera_pos[2] -= 1;
			break;
			
		case 4: // mouse wheel down
			camera_pos[2] += 1;
			break;
			
		default:
			arcball_on = false;
			break;
	}
}

void mouseMotion(int x, int y) {
	if (arcball_on) {
		cur_mpos[0] = x;
		cur_mpos[1] = y;
	}
}

Vec3 getArcballVector(int x, int y) {
	Vec3 point = Vec3((float)x/SCREEN_WIDTH - 0.5, 0.5 - (float)y/SCREEN_HEIGHT, 0);
	
	float hyp_sqr = point.x*point.x + point.y*point.y;
	
	if (hyp_sqr <= 1) {
		// mouse is inside arcball
		point.z = std::sqrt(1.0 - hyp_sqr);
	} else {
		// mouse is outside arcball
		point = Vec3::normalize(point);
	}
	return point;
}

void drawTriangleStrip(const VertexBuffer& buffer) {
	glColor3f(1, 1, 1);
	glBegin(GL_TRIANGLE_STRIP);
	for(int i = 0; i < (int)buffer.size(); ++i) {
		const Vec3& v = buffer[i];
		glVertex3f(v.x, v.y, v.z);
	}
	glEnd();
}

// GLUT callback to handle rendering the scene
void renderScene(void)
{
	//rotation
	static Matrix rotation_matrix;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,   // FOV y
					1.0f, 	// Aspect ratio (width/height)
					0.1f, 	// Near distance
					1000.0f);// Far distance

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt( camera_pos[0], camera_pos[1], camera_pos[2],    // Eye position
			   0, 0, 0, 	// Lookat position
  			   0, 1, 0 );	// Up vector

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPushMatrix();
	{
		// calculate arcball rotation
		if (arcball_on && ((last_mpos[0] != cur_mpos[0]) || (last_mpos[1] != cur_mpos[1]))) {
			Vec3 m1 = getArcballVector(last_mpos[0], last_mpos[1]);
			Vec3 m2 = getArcballVector(cur_mpos[0], cur_mpos[1]);
			float angle = 2 * std::acos(std::min(1.0f, Vec3::dotProduct(m1, m2)));
			Vec3 axis = Vec3::normalize(Vec3::crossProduct(m1, m2));
			
			Matrix new_rotation = calcRotationMatrix(angle, axis);
			rotation_matrix = rotation_matrix * new_rotation;
			
			for(int i = 0; i < 2; i++) {
				last_mpos[i] = cur_mpos[i];
			}
		}
		
		// glRotatef(rotation_angle * 180 / M_PI, rotation_axis.x, rotation_axis.y, rotation_axis.z);
		glMultMatrixf(rotation_matrix.data);

		if (show_spine) {
			glColor3f(1, 1, 1);
			glBegin(GL_LINES);
			for(int i = 0; i < (int)spine.size(); i++) {
				const Vec3& v = spine[i];
				glVertex3f(v.x, v.y, v.z);
			}
			glEnd();
		}

		if (show_girth) {
			for(int i = 0; i < (int)cylinder_list.size(); i++) {
				drawTriangleStrip(cylinder_list[i]);
			}
		}

		// drawFractal(fractal, initial_state, fractal_scale, angle_increment);
	}
	glPopMatrix();

	glutSwapBuffers();
}

bool parseLSystemFile(const char* filename) {
	std::ifstream fin(filename);
	while(!fin.eof() && fin.good()) {
		std::string str;

		fin >> str;
		if (str.size() > 0) {
			if (str[0] == '#') {
				std::string garbage;
				std::getline(fin, garbage);
			} else {
				std::string op;
				fin >> op;
				if (op == "=") {
					if (str == "angle") {
						fin >> angle_increment;
						angle_increment *= M_PI / 180.0;
					} else if (str == "girth") {
						fin >> girth;
					} else {
						std::cout << "Invalid variable: " << str << '\n';
					}
				} else if (op == "->") {
					if (str.size() != 1) {
						std::cout << "rule " << str << " must be one character\n";
					} else {
						std::string rule;
						std::getline(fin, rule);
						if (rule.size() == 0) {
							std::cout << "Warning: rule for " << str << " is empty\n";
						}
						grammar[str[0]] = rule;
					}
				} else {
					std::cout << "Invalid operation: " << op << '\n';
				}
			}
		}
	}
	return fin.eof();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	if (argc == 1) {
		std::cout << "usage is:   " << argv[0] << " L-system-file\n";
		return 1;
	}

	if (!parseLSystemFile(argv[1])) {
		std::cout << "Error: could not parse file\n";
		return 1;
	}

	generateFractal();

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowPosition(100,100);
	glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
	glutCreateWindow("Assignment 4 - Eric Roberts");

	glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
  	glEnable(GL_DEPTH_TEST);
  	glEnable(GL_NORMALIZE);

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutMouseFunc(mouseAction);
	glutMotionFunc(mouseMotion);
	glutKeyboardFunc(keyboardInput);
	glutSpecialFunc(specialKeyboardInput);

	glutMainLoop();

	return 0;
}