#include "main.h"
#include <iostream>
#include <vector>
#include "lodepng.h"
#include <fstream>
#include <bitset>
#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;
using namespace cv;

const string directory = "images/";
const string outputDirectory = "output_images/";
unsigned int width = 1920;
unsigned int height = 1080;
unsigned int pixelSize = 4;
unsigned const int numBytes = ((width / pixelSize) * (height / pixelSize)) / 4;
bool endFile = false;
unsigned int numPNG = 0;
const string fileToEncode = "dreamy.mp4";
const string outputVideo = "videos/output.mp4";
const string outputDecode = "testfiles/decoded";
const int framesPerImage = 1;
const string outfileExt = ".mp4";
const int tolerance = 150;

/**
* Main function of execution, propmts user for decoding/encoding
*/
int main() {
	char input;
	cout << "Do you want to encode (e) or decode(d) or both (b) : ";
	input = 'd';
	cin >> input;

	if (input == 'e') {
		encode();
	}
	else if (input == 'd') {
		decode();
	}
	else if (input == 'b') {
		encode();
		decode();
	}
	else {
		cout << "Invalid input" << endl;
	}

	return 0;
}

/**
* Generates a PNG from an image vector as outputName
* 
* @param image a vector of form {r,g,b,a,r,g,...}
* @param outputName the output name of the PNG
* @return 1 if successful
*/
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

/**
* Generates an immage array from an array of bytes (colors depend on bytes)
*
* @param bytes a vector of bytes
* @return a vector of form {r,g,b,a,r,g,...}
*/
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

/**
* Returns the nth set of bytes of size numBytes (number of bytes that will fit within the specified resolution)
*
* @param n the set of bytes to return (0 will return 1st png worth and so on)
* @param fileName the file from which to read the bytes from
* @return a vector of bytes from the specified reigion of the file
*/
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

/**
* Reads from the directory of generated PNGs from functions above and stitches
* them together in a video of speficied format, frame rate, etc.
*/
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

/**
* The function to call for encoding a file
*/
void encode() {
	vector<unsigned char> image;
	vector<unsigned char> bytes;

	string filename = "testfiles/" + fileToEncode;
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
}

/**
* The function to call for decoding a file
*/
void decode(){
	vector<unsigned char> bytes;
	filesystem::remove_all(outputDirectory);
	generatePNGSequence("videos/output.mp4");
	filesystem::remove("testfiles/outfile" + outfileExt);
	string picName;
	for (int i = 0; i < numPNG; i++) {
		picName = "output_images/" + to_string(i) + ".png";
		bytes = PNGToData(picName);
		appendBytesToFile(bytes, "testfiles/outfile" + outfileExt);
	}
	//bytes = PNGToData("output_images/0.png");
	/*for (auto x : bytes) {
		cout << x ;
	}*/
}

/**
* Generates a sequence of PNGs from a specified video path
*
* @param videoPath the path of the video
*/
void generatePNGSequence(string videoPath) {

	VideoCapture video(videoPath);
	if (!video.isOpened())
	{
		cerr << "Error opening video file: " << videoPath << endl;
		return;
	}

	int frameCount = static_cast<int>(video.get(CAP_PROP_FRAME_COUNT));
	int frameNumber = 0;

	// Create the output directory if it doesn't exist
	filesystem::create_directory(outputDirectory);

	while (frameNumber < frameCount)
	{
		Mat frame;
		if (!video.read(frame))
		{
			cerr << "Error reading frame " << frameNumber << " from video." << endl;
			break;
		}

		string outputName = outputDirectory + to_string(frameNumber) + ".png";
		if (!imwrite(outputName, frame))
		{
			cerr << "Error saving frame " << frameNumber << " as PNG." << endl;
		}

		frameNumber++;
	}

	video.release();

	cout << "PNG sequence generated successfully. Total frames: " << frameNumber << endl;
	numPNG = frameNumber;
}

/**
* Converts a PNG from the generatePNGSequence back into a vector of bytes
*
* @param pngImagePath the path to the png image
* @return a vector of bytes correpsonding to the png image
*/
vector<unsigned char> PNGToData(string pngImagePath) {
	vector<unsigned char> bytes;
	unsigned char byte = 0;
	Mat image = imread(pngImagePath);
	if (image.empty())
	{
		cerr << "Error reading image: " << pngImagePath << endl;
		return bytes;
	}

	Vec3b color;
	int byteCounter = 0;
	for (int y = pixelSize / 2; y < image.rows; y += pixelSize) {
		for (int x = pixelSize / 2; x < image.cols; x += pixelSize) {
			color = image.at<Vec3b>(y, x);
			byte = byte << 2;
			//White marks the end of the file
			if (static_cast<int>(color[0]) > tolerance && static_cast<int>(color[1]) > tolerance && static_cast<int>(color[2]) > tolerance) {
				return bytes;
			}
			//Red, 01
			else if (static_cast<int>(color[2]) > tolerance) { byte |= 1; }
			//Green, 11
			else if (static_cast<int>(color[0]) > tolerance) { byte |= 3; }
			//Blue, 10
			else if (static_cast<int>(color[1]) > tolerance) { byte |= 2; }
			//Black, 00, automatically occurs in byte from the double bit shift

			if (byteCounter == 3) {
				bytes.push_back(byte);
				byteCounter = 0;
			}
			else { byteCounter++; }
		}
	}
	return bytes;
}

/**
* Appends bytes to the end of a specified file
*
* @param bytes the bytes to append to a file
* @param filename the file name of which to append the bytes to
*/
void appendBytesToFile(const vector<unsigned char>& bytes, const string& filename)
{
	// Open the file in binary append mode to add bytes at the end
	ofstream file(filename, ios::binary | ios::app);
	if (!file)
	{
		cerr << "Error opening file: " << filename << endl;
		return;
	}

	// Write the bytes to the file
	file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	if (!file)
	{
		cerr << "Error writing bytes to file: " << filename << endl;
	}

	file.close();
}
