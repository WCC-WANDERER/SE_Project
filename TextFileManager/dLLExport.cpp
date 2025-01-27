#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include "FileManager.cpp"


struct FileComparisonResult {
    char* file1ReturnContent;
    char* file2ReturnContent;
    char* differences;
};


// Function to check if the file is already in .txt format
bool isTxtFile(const std::string& filePath) {

    return filePath.substr(filePath.find_last_of(".") + 1) == "txt";
}

// Function to convert .doc, .docx, or .odt to .txt using LibreOffice's soffice command
std::string convertToTxt(const std::string& inputFilePath, const std::string& outputDir) {

    // Convert input file path to absolute path
    std::filesystem::path inputAbsPath = std::filesystem::absolute(inputFilePath);

    // Convert outputDir to a std::filesystem::path
    std::filesystem::path outputDirPath(outputDir);

    // Extract the file name without extension
    std::filesystem::path inputPath(inputAbsPath);
    std::string outputFileName = inputPath.stem().string() + ".txt";

    // Combine the output directory and the output file name
    std::filesystem::path outputFilePath = outputDirPath / outputFileName;

    // Construct the Pandoc command with --wrap=none
    std::string command = "pandoc --to=plain+smart --wrap=none \"" + inputAbsPath.string() + "\" -o \"" + outputFilePath.string() + "\"";

    // Execute the command
    int result = system(command.c_str());

    // Check for errors during conversion
    if (result != 0) {
        std::cerr << "Error converting file: " << inputAbsPath.string() << std::endl;
        throw std::runtime_error("Failed to convert file: " + inputAbsPath.string());
    }
    else {
        std::cout << "File successfully converted to: " << outputFilePath.string() << std::endl;
    }

    //// Post-process the output file to remove empty lines
    //std::ifstream inputFile(outputFilePath.string());
    //std::ofstream tempFile("temp_output.txt");

    //std::string line;
    //while (std::getline(inputFile, line)) {
    //    if (!line.empty()) {  // Only write non-empty lines to the new file
    //        tempFile << line << "\n";
    //    }
    //}

    //inputFile.close();
    //tempFile.close();

    //// Replace the original output file with the cleaned file
    //std::remove(outputFilePath.string().c_str());
    //std::rename("temp_output.txt", outputFilePath.string().c_str());

    // Return the output file path as a string
    return outputFilePath.string();
}


extern "C" {

    __declspec(dllexport) FileComparisonResult CompareFiles(const char* file1Path, const char* file2Path) {

        FileComparisonResult result;

        try {

            // Check if the extension is .doc or .odt file, convert if necessary
            std::string outputDir = std::filesystem::current_path().string();

            bool test = true;
            bool isFile1Converted = false;
            bool isFile2Converted = false;

            // Convert file1Path if not a .txt file
            std::string file1ConvertedPath = file1Path;
            if (!isTxtFile(file1ConvertedPath)) {
                file1ConvertedPath = convertToTxt(file1ConvertedPath, outputDir);
                isFile1Converted = true;
            }

            // Convert file2Path if not a .txt file
            std::string file2ConvertedPath = file2Path;
            if (!isTxtFile(file2ConvertedPath)) {
                file2ConvertedPath = convertToTxt(file2ConvertedPath, outputDir);
                isFile2Converted = true;
            }

            File firstFile(file1ConvertedPath);
            File secondFile(file2ConvertedPath);

            std::vector<std::string> firstFileContent = firstFile.retrieveContent();
            std::vector<std::string> secondFileContent = secondFile.retrieveContent();

            // Prepare original content buffers (full content of both files)
            std::ostringstream file1ContentStream;
            for (const auto& line : firstFileContent) {
                file1ContentStream << line << "\n";
            }
            std::ostringstream file2ContentStream;
            for (const auto& line : secondFileContent) {
                file2ContentStream << line << "\n";
            }

            std::string file1ContentString = file1ContentStream.str();
            std::string file2ContentString = file2ContentStream.str();

            // Allocate memory for original content and differences
            result.file1ReturnContent = new char[file1ContentString.length() + 1];
            result.file2ReturnContent = new char[file2ContentString.length() + 1];

            // Copy the strings into the allocated memory
            std::copy(file1ContentString.begin(), file1ContentString.end(), result.file1ReturnContent);
            std::copy(file2ContentString.begin(), file2ContentString.end(), result.file2ReturnContent);

            // Null-terminate the strings
            result.file1ReturnContent[file1ContentString.length()] = '\0';
            result.file2ReturnContent[file2ContentString.length()] = '\0';

            Comparator comparator;
            std::vector<Difference> differences = comparator.compareFiles(firstFileContent, secondFileContent);

            // Assuming differences has been populated correctly.
            std::ostringstream diffStream;
            for (const auto& diff : differences) {
                diffStream << "Line " << diff.getLineNumber() << ": File1 -> " << diff.getFirstFileContent()
                    << ", File2 -> " << diff.getSecondFileContent() << "\n";
            }

            std::string diffString = diffStream.str();
            result.differences = new char[diffString.length() + 1];

            // Copy the differences into allocated memory
            std::copy(diffString.begin(), diffString.end(), result.differences);
            result.differences[diffString.length()] = '\0';

            return result;
        }
        catch (const std::exception& ex) {
            std::cerr << "Exception in CompareFiles: " << ex.what() << std::endl;
            return {}; // Return empty struct in case of error
        }
    }

    // Function to free the memory allocated for the result string
    __declspec(dllexport) void FreeMemory(char* ptr) {
        if (ptr != nullptr) {
            delete[] ptr;  // Free the memory allocated for the result string
        }
    }


    /*__declspec(dllexport) int CompareFiles(const char* file1Path, const char* file2Path, char* resultBuffer, int resultBufferSize, 
    char* file1ContentBuffer, int file1ContentBufferSize, char* file2ContentBuffer, int file2ContentBufferSize) {

        try {

            // Check if the extension is .doc or .odt file, convert if necessary
            std::string outputDir = std::filesystem::current_path().string();

            bool test = true;
            bool isFile1Converted = false;
            bool isFile2Converted = false;

            // Convert file1Path if not a .txt file
            std::string file1ConvertedPath = file1Path;
            if (!isTxtFile(file1ConvertedPath)) {
                file1ConvertedPath = convertToTxt(file1ConvertedPath, outputDir);
                isFile1Converted = true;
            }

            // Convert file2Path if not a .txt file
            std::string file2ConvertedPath = file2Path;
            if (!isTxtFile(file2ConvertedPath)) {
                file2ConvertedPath = convertToTxt(file2ConvertedPath, outputDir);
                isFile2Converted = true;
            }

            File firstFile(file1ConvertedPath);
            File secondFile(file2ConvertedPath);

            std::vector<std::string> firstFileContent = firstFile.retrieveContent();
            std::vector<std::string> secondFileContent = secondFile.retrieveContent();

            // Prepare original content buffers (full content of both files)
            std::ostringstream originalContentStream;
            for (const auto& line : firstFileContent) {
                originalContentStream << line << "\n";
            }
            std::ostringstream secondOriginalContentStream;
            for (const auto& line : secondFileContent) {
                secondOriginalContentStream << line << "\n";
            }

            std::string originalContentString = originalContentStream.str();
            std::string secondOriginalContentString = secondOriginalContentStream.str();

            // Ensure buffer sizes are large enough
            if (originalContentString.size() >= file1ContentBufferSize || secondOriginalContentString.size() >= file2ContentBufferSize) {
                throw std::runtime_error("Buffer size too small");
            }

            // Copy the full original content to the buffers
            std::copy(originalContentString.begin(), originalContentString.end(), file1ContentBuffer);
            std::copy(secondOriginalContentString.begin(), secondOriginalContentString.end(), file2ContentBuffer);
            file1ContentBuffer[originalContentString.size()] = '\0';
            file2ContentBuffer[secondOriginalContentString.size()] = '\0';

            Comparator comparator;
            std::vector<Difference> differences = comparator.compareFiles(firstFileContent, secondFileContent);

            // Build the string of differences
            std::ostringstream diffStream;
            for (const auto& diff : differences) {
                diffStream << "Line " << diff.getLineNumber() << ": File1 -> " << diff.getFirstFileContent()
                    << ", File2 -> " << diff.getSecondFileContent() << "\n";
            }

            std::string diffString = diffStream.str();
            if (diffString.size() >= resultBufferSize) {
                throw std::runtime_error("Buffer size too small");
            }
            std::copy(diffString.begin(), diffString.end(), resultBuffer);
            resultBuffer[diffString.size()] = '\0';

            return 0; // Success

        }
        catch (const std::exception& ex) {
            std::cerr << "Exception in CompareFiles: " << ex.what() << std::endl;
            return -1;  // Indicate failure
        }
    }*/
}


