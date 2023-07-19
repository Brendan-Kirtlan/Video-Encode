#include "main.h"
#include <iostream>
#include <vector>
#include "lodepng.h"
#include <fstream>
#include <bitset>
#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;

const string directory = "images/";
unsigned int width = 1920;
unsigned int height = 1080;
unsigned int pixelSize = 4;
unsigned const int numBytes = ((width / pixelSize) * (height / pixelSize)) / 4;
bool endFile = false;
unsigned int numPNG = 0;
const string outputVideo = "videos/output.mp4";
const int framesPerImage = 2;

int main() {
	vector<unsigned char> image;
	vector<unsigned char> bytes;

	string filename = "testfiles/Capture.PNG";

	//clean image directory
	filesystem::remove_all(directory);
	filesystem::create_directory(directory);
	while (!endFile) {
		string outputName = to_string(numPNG) + ".png";
		bytes = getNthSet(numPNG, filename);
		image = generateImageArray(bytes);
		generatePNG(image, outputName);
		numPNG++;
	}
	cout << numPNG << " images saved" << endl;
	generateVideo();
	return 0;
}

int generatePNG(vector<unsigned char> image, string outputName)
{
	const string outputPath = directory + outputName;
	if (lodepng::encode(outputPath, image, width, height) != 0) {
		cout << "Error encoding PNG" << endl;
		return 1;
	}

	//cout << "Image saved as " << outputName << endl;

	return 0;

	//if there's an error, display it
	//if (error) cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

vector<unsigned char> generateImageArray(vector<unsigned char> bytes) {
	vector<unsigned char> image(width * height * 4);
	int gridPos = 0;
	for (unsigned int y = 0; y < height; ++y) {
		for (unsigned int x = 0; x < width; ++x) {
			unsigned int pixelIndex = (y * width + x) * 4;

			//Grid pixel position, pixels are pixelSize*pixelSize in dimension
			gridPos = (x / pixelSize) + (width / pixelSize) * (y / pixelSize);

			//Black and white version, need to change numBytes to account for more space taken up
			/*if ((gridPos / 8) < bytes.size()) {
				if (bytes[gridPos / 8] >> (7 - (gridPos % 8)) & 1) {
					image[pixelIndex] = 0;
					image[pixelIndex + 1] = 0;
					image[pixelIndex + 2] = 0;
					image[pixelIndex + 3] = 255;
				}
				else {
					image[pixelIndex] = 255;
					image[pixelIndex + 1] = 255;
					image[pixelIndex + 2] = 255;
					image[pixelIndex + 3] = 255;
				}
			}
			else {
				image[pixelIndex] = 255;
				image[pixelIndex + 1] = 0;
				image[pixelIndex + 2] = 255;
				image[pixelIndex + 3] = 255;
			}*/

			int bits = 0;
			if ((gridPos / 4) < bytes.size()) {
				//Takes rightmost 2 bits after bit shifting accounting for grid position
				bits = bytes[gridPos / 4] >> (6 - ((gridPos % 4) * 2)) & 3;
				switch (bits) {
				case 0:
					image[pixelIndex] = 0;
					image[pixelIndex + 1] = 0;
					image[pixelIndex + 2] = 0;
					image[pixelIndex + 3] = 255;
					break;
				case 1:
					image[pixelIndex] = 255;
					image[pixelIndex + 1] = 0;
					image[pixelIndex + 2] = 0;
					image[pixelIndex + 3] = 255;
					break;
				case 2:
					image[pixelIndex] = 0;
					image[pixelIndex + 1] = 255;
					image[pixelIndex + 2] = 0;
					image[pixelIndex + 3] = 255;
					break;
				case 3:
					image[pixelIndex] = 0;
					image[pixelIndex + 1] = 0;
					image[pixelIndex + 2] = 255;
					image[pixelIndex + 3] = 255;
					break;
				}
			}
			else {
				image[pixelIndex] = 255;
				image[pixelIndex + 1] = 255;
				image[pixelIndex + 2] = 255;
				image[pixelIndex + 3] = 255;
			}

			//Grid lines
			/*if (x % pixelSize == pixelSize - 1 || y % pixelSize == pixelSize - 1) {
				image[pixelIndex] = 255;
				image[pixelIndex + 1] = 255;
				image[pixelIndex + 2] = 255;
				image[pixelIndex + 3] = 255;
			}*/
		}
	}
	return image;
}

vector<unsigned char> getNthSet(unsigned int n, string fileName) {
	unsigned int bytesToUse = numBytes;
	vector<unsigned char> result;

	ifstream file(fileName, ios::binary);
	if (!file) {
		cerr << "Failed to open file: " << fileName << endl;
		return result;
	}

	// Determine the file size
	file.seekg(0, ios::end);
	streampos fileSize = file.tellg();
	file.seekg(0, ios::beg);

	// Calculate the starting position of the nth set
	streampos startPos = n * bytesToUse;

	// Adjust bytesToUse if necessary
	if (static_cast<streampos>(startPos) + static_cast<streampos>(bytesToUse) > fileSize) {
		bytesToUse = fileSize - startPos;
		endFile = true;
	}

	// Seek to the desired position
	file.seekg(startPos, ios::beg);

	// Read the bytes into the result vector
	vector<unsigned char> buffer(bytesToUse);
	file.read(reinterpret_cast<char*>(buffer.data()), bytesToUse);
	if (!file) {
		cerr << "Failed to read bytes from file." << endl;
		return result;
	}

	// Copy the read bytes into the result vector
	result.assign(buffer.begin(), buffer.end());

	return result;
}

void generateVideo() {
	cv::VideoWriter video(outputVideo, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(width, height));

	if (!video.isOpened()) {
		cerr << "Failed to create video file: " << outputVideo << endl;
		return;
	}

	cv::Mat frame;
	for (int i = 0; i < numPNG; i++) {
		string imagePath = directory + to_string(i) + ".png";
		frame = cv::imread(imagePath);

		if (frame.empty())
			break;

		for (int j = 0; j < framesPerImage; j++)
			video.write(frame);
	}

	video.release();
	cout << "Video created successfully: " << outputVideo << endl;
}

