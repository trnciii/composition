#pragma once

#include <string>
#include <fstream>

template<typename T>
bool writeVector(const std::vector<T>& v, const std::string& name) {
	std::ofstream out(name, std::ios::out | std::ios::binary);
	if (!out) return false;

	int count = v.size();
	out.write(reinterpret_cast<const char*>(&count), sizeof(count));
	out.write(reinterpret_cast<const char*>(&v[0]), v.size() * sizeof(T));
	out.close();

	return true;
}

template<typename T>
bool readVector(std::vector<T>& v, const std::string& name) {
	v.clear();
	int count;

	std::ifstream in(name, std::ios::in | std::ios::binary);
	if ( !in ) return false;
	
	in.read(reinterpret_cast<char*>(&count), sizeof(count));
	v.resize(count);
	in.read(reinterpret_cast<char*>(&v[0]), count * sizeof(T));
	in.close();

	return true;
}