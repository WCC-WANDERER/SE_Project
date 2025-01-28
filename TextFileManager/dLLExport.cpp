#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <windows.h>
#include "FileManager.cpp"


struct FileComparisonResult {
    char* file1ReturnContent;
    char* file2ReturnContent;
    char* differences;
};

// Function to check if the file is already in .txt format
//bool isTxtFile(const std::string& filePath) {
//
//    return filePath.substr(filePath.find_last_of(".") + 1) == "txt";
//}

// Function to delete temporary files by setting attributes to normal
void deleteTemporaryFile(const std::string& outputFilePath) {

    // Set file attributes to normal (if it exists)
    if (std::filesystem::exists(outputFilePath)) {
        if (!SetFileAttributes(std::filesystem::path(outputFilePath).c_str(), FILE_ATTRIBUTE_NORMAL)) {
            DWORD error = GetLastError();
            std::cerr << "Failed to set file attributes to normal for: " << outputFilePath
                << ". Error Code: " << error << std::endl;
        }

        // Delete the original file
        try {
            std::filesystem::remove(outputFilePath);
        }
        catch (const std::exception& ex) {
            std::cerr << "Error deleting file: " << ex.what() << std::endl;
            return;
        }
    }
    else {
        std::cerr << "File does not exist: " << outputFilePath << std::endl;
        return;
    }
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

    // Temporary file for cleaned content
    const std::string tempFilePath = "temp_output.txt";

    // Open the original output file and the temporary file
    std::ifstream inputFile(outputFilePath);
    std::ofstream tempFile(tempFilePath);

    if (!inputFile.is_open() || !tempFile.is_open()) {
        std::cerr << "Error: Unable to open files for processing." << std::endl;
        throw std::runtime_error("File not found: " + outputFilePath.string() + " or " + tempFilePath);
    }

    // Remove empty lines from the file
    std::string line;
    while (std::getline(inputFile, line)) {
        if (!line.empty()) {  // Only write non-empty lines
            tempFile << line << "\n";
        }
    }

    // Close the files
    inputFile.close();
    tempFile.close();

    deleteTemporaryFile(outputFilePath.string());

    // Rename the temporary file to the original file name
    try {
        std::filesystem::rename(tempFilePath, outputFilePath);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error renaming temporary file: " << ex.what() << std::endl;
    }

    // Return the output file path as a string
    return outputFilePath.string();

}


extern "C" {

    __declspec(dllexport) FileComparisonResult CompareFiles(const char* file1Path, const char* file2Path) {

        FileComparisonResult result;
        std::string file1Str, file2Str;
        std::string file1ConvertedPath, file2ConvertedPath;
        bool isFile1Converted = false;
        bool isFile2Converted = false;

        try {
            // Convert files to text if needed (keep your existing conversion logic here)
            std::string outputDir = std::filesystem::current_path().string();

            File firstFile(file1Path);
            File secondFile(file2Path);

            // Convert file1
            file1ConvertedPath = firstFile.getPath();
            if (firstFile.getExtension() != ".txt") {
                file1ConvertedPath = convertToTxt(file1ConvertedPath, outputDir);
                isFile1Converted = true;
            }

            // Convert file2
            file2ConvertedPath = secondFile.getPath();
            if (secondFile.getExtension() != ".txt") {
                file2ConvertedPath = convertToTxt(file2ConvertedPath, outputDir);
                isFile2Converted = true;
            }

            //// Open files
            //std::ifstream file1(file1ConvertedPath);
            //std::ifstream file2(file2ConvertedPath);
            //if (!file1) throw std::runtime_error("File not found: " + file1ConvertedPath);
            //if (!file2) throw std::runtime_error("File not found: " + file2ConvertedPath);

            ////Prepare content buffers and differences
            //std::ostringstream file1Content, file2Content;
            //std::vector<Difference> differences;
            //int lineNumber = 0;


            //while (true) 
            //{
            //    std::string line1, line2;
            //    bool gotLine1 = static_cast<bool>(std::getline(file1, line1));
            //    bool gotLine2 = static_cast<bool>(std::getline(file2, line2));
            //    lineNumber++;

            //    if (!gotLine1 && !gotLine2)
            //    {
            //        break;
            //    }

            //    std::string currentLine1 = gotLine1 ? line1 : "";
            //    std::string currentLine2 = gotLine2 ? line2 : "";

            //    //Compare and store
            //    if (currentLine1 != currentLine2) {
            //        differences.emplace_back(lineNumber, currentLine1, currentLine2);
            //    }

            //    if (gotLine1) file1Content << line1 << "\n";
            //    if (gotLine2) file2Content << line2 << "\n";
            //}

            //// Close the files
            //file1.close();
            //file2.close();

            ////Store file contents
            //std::string file1Str = file1Content.str();
            //std::string file2Str = file2Content.str();

            Comparator comparator;
            comparator.compareFiles(file1ConvertedPath, file2ConvertedPath, file1Str, file2Str);

            result.file1ReturnContent = new char[file1Str.size() + 1];
            std::copy(file1Str.begin(), file1Str.end(), result.file1ReturnContent);
            result.file1ReturnContent[file1Str.size()] = '\0';

            result.file2ReturnContent = new char[file2Str.size() + 1];
            std::copy(file2Str.begin(), file2Str.end(), result.file2ReturnContent);
            result.file2ReturnContent[file2Str.size()] = '\0';

            //Store differences
            std::ostringstream diffStream;
            for (const auto& diff : comparator.getDifferences()) {
                diffStream << "Line " << diff.getLineNumber()
                    << ": File1 -> " << diff.getFirstFileContent()
                    << ", File2 -> " << diff.getSecondFileContent() << "\n";
            }

            std::string diffStr = diffStream.str();
            result.differences = new char[diffStr.size() + 1];
            std::copy(diffStr.begin(), diffStr.end(), result.differences);
            result.differences[diffStr.size()] = '\0';

            //Cleanup temporary files
            if (isFile1Converted) deleteTemporaryFile(file1ConvertedPath);
            if (isFile2Converted) deleteTemporaryFile(file2ConvertedPath);

            return result;

        }
        catch (const std::exception& ex) {
            // Cleanup on error
            if (isFile1Converted) deleteTemporaryFile(file1ConvertedPath);
            if (isFile2Converted) deleteTemporaryFile(file2ConvertedPath);
            std::cerr << "Error: " << ex.what() << std::endl;
            return {};
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


