#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
//#include "Source.cpp"




// Function to check if the file is already in .txt format
bool isTxtFile(const std::string& filePath) {

    return filePath.substr(filePath.find_last_of(".") + 1) == "txt";
}

// Function to convert .doc, .docx, or .odt to .txt using LibreOffice's soffice command
void convertToTxt(const std::string& inputFilePath, const std::string& outputFilePath) {

    std::string command = "soffice --headless --convert-to txt:Text " + inputFilePath + " --outdir " + outputFilePath;
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Error converting file: " << inputFilePath << std::endl;
    }
    else {
        std::cout << "File successfully converted to: " << outputFilePath << std::endl;
    }
}




extern "C" {

    __declspec(dllexport) void RetrieveContent(const char* path, char* buffer, int bufferSize) {

        std::ifstream file(path);
        if (!file) {
            throw std::runtime_error("File not found");
        }
        if (file.peek() == std::ifstream::traits_type::eof()) {
            throw std::runtime_error("File is empty");
        }

        std::ostringstream contentStream;
        std::string line;
        while (std::getline(file, line)) {
            contentStream << line << "\n";
        }
        file.close();

        std::string content = contentStream.str();
        if (content.size() >= bufferSize) {
            throw std::runtime_error("Buffer size too small");
        }
        std::copy(content.begin(), content.end(), buffer);
        buffer[content.size()] = '\0';
    }


    __declspec(dllexport) int CompareFiles(const char* file1Path, const char* file2Path, char* resultBuffer, int bufferSize) {

        // Check if the extension is .doc or .odt file, convert if necessary
        std::string outputDir = std::filesystem::current_path().string();
        std::string outputFilePath = outputDir + "converted.txt";

        if (!isTxtFile(file1Path)) {

            convertToTxt(file1Path, outputDir);
        }
        if (!isTxtFile(file2Path)) {

            convertToTxt(file2Path, outputDir);
        }

        std::ifstream file1(file1Path), file2(file2Path);
        if (!file1 || !file2) {
            throw std::runtime_error("One or both files not found");
        }

        std::vector<std::string> file1Lines, file2Lines;
        std::string line;
        while (std::getline(file1, line)) file1Lines.push_back(line);
        while (std::getline(file2, line)) file2Lines.push_back(line);

        size_t maxLines = std::max(file1Lines.size(), file2Lines.size());
        std::ostringstream diffStream;

        for (size_t i = 0; i < maxLines; i++) {
            std::string line1 = i < file1Lines.size() ? file1Lines[i] : "";
            std::string line2 = i < file2Lines.size() ? file2Lines[i] : "";
            if (line1 != line2) {
                diffStream << "Line " << (i + 1) << ": File1 -> " << line1 << ", File2 -> " << line2 << "\n";
            }
        }

        std::string differences = diffStream.str();
        if (differences.size() >= bufferSize) {
            throw std::runtime_error("Buffer size too small");
        }
        std::copy(differences.begin(), differences.end(), resultBuffer);
        resultBuffer[differences.size()] = '\0';

        return 0; // Success
    }
}







//// Function to process the converted .txt file line by line
//void processTxtFile(const std::string& txtFilePath) {
//    std::ifstream file(txtFilePath);
//    if (!file.is_open()) {
//        std::cerr << "Error opening file: " << txtFilePath << std::endl;
//        return;
//    }
//
//    std::string line;
//    while (std::getline(file, line)) {
//        // Process each line (e.g., display it, modify it, etc.)
//        std::cout << "Processing: " << line << std::endl;
//    }
//
//    file.close();
//}
