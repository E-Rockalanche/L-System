#ifndef LIGHT_HPP
#define LIGHT_HPP

struct Light {
	float specular[4];
	float diffuse[4];
	float ambient[4];
	float position[4];

	Light(const float s[4], const float d[4], const float a[4], const float p[4]) {
		for(int i = 0; i < 4; ++i) {
			specular[i] = s[i];
			diffuse[i] = d[i];
			ambient[i] = a[i];
			position[i] = p[i];
		}
	}
};

#endif