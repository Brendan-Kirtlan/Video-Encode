#pragma once
#include "main.h"
#include <iostream>
#include <vector>
#include "lodepng.h"
#include <fstream>
#include <bitset>
using namespace std;
int generatePNG(vector<unsigned char> image, string outputName);
vector<unsigned char> generateImageArray(vector<unsigned char> bytes);
vector<unsigned char> getNthSet(unsigned int n, string fileName);
void generateVideo();
void encode();
void decode();
void generatePNGSequence(string videoPath);
vector<unsigned char> PNGToData(string pngImagePath);
