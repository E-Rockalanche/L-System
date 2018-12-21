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
#include <cctype>

#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/glut.h"

#include "vec3.hpp"
#include "matrix.hpp"
#include "light.hpp"
#include "stb_image.h"

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

typedef std::map<char, std::string> Grammar;

// L-system variables
struct State {
	Vec3 position;
	float radius;
	float length;
	Vec3 axis[3];

	State(Vec3 p, float r, float l) {
		position = p;
		radius = r;
		length = l;
		for (int i = 0; i < 3; i++) {
			Vec3 v;
			v[i] = 1;
			axis[i] = v;
		}
	}
};
std::string fractal = "X";
int fractal_depth = 0;

// file variables
Grammar grammar;
std::string initial_axoim = "X";
float pitch = 0.0;
float yaw = 0.0;
float roll = 0.0;
float base_radius = 0.5;
float radius_scale = 0.5;
float base_length = 10.0;
float length_scale = 0.5;
int texture_handle = 0;

// arcball variables
int last_mpos[2];
int cur_mpos[2];
bool arcball_on = false;
float camera_pos[3] = { 0.0, 0.0, 80.0 };

// render variables
typedef std::vector<Vec3> VertexBuffer;
struct Object {
	VertexBuffer vertices;
	VertexBuffer normals;
	std::vector<float> tex_coords;

	Object() {}
};
std::vector<Object> branches;
VertexBuffer spine;
bool show_girth = true;
bool show_spine = false;
bool show_texture = true;

Light light((const float[4]){1, 1, 1, 1},
	(const float[4]){1, 1, 1, 1},
	(const float[4]){0.1, 0.1, 0.1, 1},
	(const float[4]){0, 0, 20, 1});

void assert(bool condition, const char* message) {
	if (!condition) {
		throw std::runtime_error(message);
	}
}

void checkGLError(const char* message) {
	GLenum error_enum = glGetError();
	if (GL_NO_ERROR != error_enum) {
		std::cout << "openGL error: " << error_enum << ", " << message << '\n';
		exit(1);
	}
}

unsigned int loadImage(std::string file, std::string path) {
	std::string filename = path + file;
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 4);
	
	if (data == NULL) {
		std::cout << "failed to load image " << filename << '\n';
		exit(1);
	}
	
	// create openGL texture
	unsigned int texture;
	glGenTextures(1, &texture);
	
	checkGLError("after gen textures");
	
	// bind texture for future operations
	glBindTexture(GL_TEXTURE_2D, texture);
	
	// give image to openGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	GLenum error_enum = glGetError();
	if (GL_NO_ERROR != error_enum) {
		std::cout << "an openGL error occurred while loading " << filename << '\n';
		exit(1);
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	stbi_image_free(data);
	return texture;
}

#define PITCH_ANGLE "pitch"
#define YAW_ANGLE "yaw"
#define ROLL_ANGLE "roll"
#define BASE_RADIUS "radius"
#define RADIUS_SCALE "rscale"
#define BASE_LENGTH "length"
#define LENGTH_SCALE "lscale"
#define TEXTURE "texture"
#define INITIAL_AXIOM "w"

#define degreeToRad(angle) ((angle) * M_PI / 180.0)

void parsePath(std::string full_path, std::string& filename, std::string& path) {
	filename.clear();
	for(int i = 0; i < (int)full_path.size(); ++i) {
		char c = full_path[i];
		filename += c;
		if (c == '/' || c == '\\') {
			path += filename;
			filename.clear();
		}
	}
}

bool parseLSystemFile(std::string file, std::string path = "") {
	parsePath(path + file, file, path);

	bool ok = true;
	std::ifstream fin(file);
	while(!fin.eof() && fin.good() && ok) {
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
					if (str == PITCH_ANGLE) {
						fin >> pitch;
						pitch = degreeToRad(pitch);
					} else if (str == YAW_ANGLE) {
						fin >> yaw;
						yaw = degreeToRad(yaw);
					} else if (str == ROLL_ANGLE) {
						fin >> roll;
						roll = degreeToRad(roll);
					} else if (str == BASE_LENGTH) {
						fin >> base_length;
					} else if (str == LENGTH_SCALE) {
						fin >> length_scale;
					} else if (str == BASE_RADIUS) {
						fin >> base_radius;
					} else if (str == RADIUS_SCALE) {
						fin >> radius_scale;
					} else if (str == INITIAL_AXIOM) {
						fin >> initial_axoim;
					} else if (str == TEXTURE) {
						std::string tex_filename;
						fin >> tex_filename;
						texture_handle = loadImage(tex_filename, path);
						if (texture_handle == 0) {
							std::cout << "Could not load texture " << tex_filename << '\n';
							ok = false;
						}
					} else {
						std::cout << "Invalid variable: " << str << '\n';
						ok = false;
					}
				} else if (op == "->") {
					if (str.size() != 1 || !std::isalpha(str[0])) {
						std::cout << "grammar rule " << str << " must be a single character\n";
						ok = false;
					} else {
						std::string rule;
						std::getline(fin, rule);
						if (rule.size() == 0) {
							std::cout << "Warning: rule for " << str << " is empty\n";
						}
						grammar[str[0]] = rule;
					}
				} else {
					std::cout << "Invalid token: " << op << '\n';
					ok = false;
				}
			}
		}
	}

	return ok && fin.eof();
}

std::string expandFractal(std::string fractal, Grammar grammar) {
	std::string result;
	for(int i = 0; i < (int)fractal.size(); ++i) {
		char c = fractal[i];
		bool replaced = false;
		if (std::isalpha(c)) {
			for(auto it = grammar.begin(); it != grammar.end(); it++) {
				if (c == it->first) {
					result += it->second;
					replaced = true;
					break;
				}
			}
		}
		if (!replaced && !std::isspace(c)) {
			result += c;
		}
	}
	return result;
}

void generateBranch(Vec3 v1, Vec3 v2, float r1, float r2, int sections = 6) {
	sections = std::max(sections, 3);

	// generate axis
	Vec3 l = v2 - v1;
	Vec3 x = l + Vec3(1, 1, 1);
	if (Vec3::crossProduct(x, l) == Vec3(0, 0, 0)) {
		x += Vec3(0, 1, 0);
	}
	x -= Vec3::project(x, l);
	x.normalize();
	Vec3 y = Vec3::crossProduct(l, x);
	y.normalize();

	// create new object
	branches.push_back(Object());
	Object& branch = branches.back();
	VertexBuffer& vertices = branch.vertices;
	VertexBuffer& normals = branch.normals;
	std::vector<float>& tex_coords = branch.tex_coords;

	// calculate vertices, normals, tex coords
	for(int i = 0; i < sections + 1; i++) {
		float angle = 2.0 * M_PI * (i % sections) / sections;
		Vec3 r = std::cos(angle) * x + std::sin(angle) * y;
		Vec3 p1 = v1 + r1 * r;
		Vec3 p2 = v2 + r2 * r;

		vertices.push_back(p2);
		vertices.push_back(p1);

		Vec3 tangent = Vec3::crossProduct(p2 - p1, r);
		Vec3 normal = Vec3::crossProduct(tangent, p2 - p1);
		normal.normalize();

		normals.push_back(normal);
		normals.push_back(normal);

		float tex_x = (float)i / (float)sections;

		tex_coords.push_back(tex_x);
		tex_coords.push_back(1.0);

		tex_coords.push_back(tex_x);
		tex_coords.push_back(0.0);
	}
}

void generateFractal() {
	fractal = initial_axoim;
	for(int i = 0; i < fractal_depth; i++) {
		fractal = expandFractal(fractal, grammar);
	}

	std::vector<State> state_stack;
	State initial_state(Vec3(0, -7, 0), base_radius, base_length);
	state_stack.push_back(initial_state);

	branches.clear();
	spine.clear();

	for(int i = 0; i < (int)fractal.size(); ++i) {
		State& cur_state = state_stack.back();
		char c = fractal[i];

		if (std::isalpha(c)) {
			// draw forward
			Vec3 p1 = cur_state.position;

			cur_state.position += cur_state.length * cur_state.axis[1];

			Vec3 p2 = cur_state.position;
			float radius1 = cur_state.radius;
			cur_state.radius *= radius_scale;
			
			// cur_state.radius *= (10.0 - fractal_scale)/10.0;

			// cur_state.radius *= 1.0 - 1.0 / std::pow(2, fractal_depth);

			generateBranch(p1, p2, radius1, cur_state.radius, 8);

			spine.push_back(p1);
			spine.push_back(p2);
		} else if (c == '[') {
			// push stack
			state_stack.push_back(cur_state);
		} else if (c == ']') {
			// pop stack
			assert(state_stack.size() > 1, "popping initial state");
			state_stack.pop_back();
		} else if (c == '*') {
			cur_state.length *= length_scale;
		} else if (!std::isspace(c)) {
			Vec3 axis;
			float angle = 0.0;
			switch(c) {
				case '+': // rotate left
					axis = cur_state.axis[2];
					angle = yaw;
					break;

				case '-': // rotate right
					axis = cur_state.axis[2];
					angle = -yaw;
					break;
					
				case '&': // pitch down
					axis = cur_state.axis[0];
					angle = -pitch;
					break;
					
				case '^': // pitch up
					axis = cur_state.axis[0];
					angle = pitch;
					break;
					
				case '\\': // roll left
					axis = cur_state.axis[1];
					angle = -roll;
					break;
					
				case '/': // roll right
					axis = cur_state.axis[1];
					angle = roll;
					break;
					
				case '|': // turn around
					axis = cur_state.axis[2];
					angle = M_PI;
					break;
					
				default: // invalid token
					std::cout << "invalid token: " << fractal[i] << '\n';
					throw std::runtime_error("invalid token");
			}
			Matrix rotation = calcRotationMatrix(angle, axis);
			for(int i = 0; i < 3; i++) {
				cur_state.axis[i] = cur_state.axis[i] * rotation;
			}
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

		case 't':
		case 'T':
			show_texture = !show_texture;
			break;
	}
}

void specialKeyboardInput(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
			fractal_depth = std::min(6, fractal_depth + 1);
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
			light.position[2] -= 1;
			break;
			
		case 4: // mouse wheel down
			camera_pos[2] += 1;
			light.position[2] += 1;
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

void drawBranch(const Object& object) {
	if (show_texture) {
		glBindTexture(GL_TEXTURE_2D, texture_handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, object.tex_coords.data());
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, object.vertices.data());

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, object.normals.data());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, object.vertices.size());

	checkGLError("after render");

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void renderScene() {
	//rotation
	static Matrix rotation_matrix;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light.specular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light.diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light.ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, light.position);

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
		
		glMultMatrixf(rotation_matrix.data);

		if (show_spine) {
			glDisable(GL_LIGHTING);

			glColor3f(1, 1, 1);

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, spine.data());

			glDrawArrays(GL_LINES, 0, spine.size());

			glDisableClientState(GL_VERTEX_ARRAY);

			glEnable(GL_LIGHTING);
		}

		if (show_girth) {
			for(int i = 0; i < (int)branches.size(); i++) {
				drawBranch(branches[i]);
			}
		}
	}
	glPopMatrix();

	glutSwapBuffers();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowPosition(100,100);
	glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
	glutCreateWindow("Assignment 4 - Eric Roberts");

	glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutMouseFunc(mouseAction);
	glutMotionFunc(mouseMotion);
	glutKeyboardFunc(keyboardInput);
	glutSpecialFunc(specialKeyboardInput);
	
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glShadeModel(GL_SMOOTH);
  	glEnable(GL_DEPTH_TEST);
  	glEnable(GL_NORMALIZE);

	if (argc == 1) {
		std::cout << "usage is:   " << argv[0] << " L-system-file\n";
		return 1;
	}

	if (!parseLSystemFile(argv[1])) {
		std::cout << "Error: could not parse file\n";
		return 1;
	}

	generateFractal();

	glutMainLoop();

	return 0;
}