#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <windows.h>

//// Function to delete temporary files by setting attributes to normal
//void deleteTemporaryFile(const std::string& outputFilePath) {
//
//    // Set file attributes to normal (if it exists)
//    if (std::filesystem::exists(outputFilePath)) {
//        if (!SetFileAttributes(std::filesystem::path(outputFilePath).c_str(), FILE_ATTRIBUTE_NORMAL)) {
//            DWORD error = GetLastError();
//            std::cerr << "Failed to set file attributes to normal for: " << outputFilePath
//                << ". Error Code: " << error << std::endl;
//        }
//
//        // Delete the original file
//        try {
//            std::filesystem::remove(outputFilePath);
//        }
//        catch (const std::exception& ex) {
//            std::cerr << "Error deleting file: " << ex.what() << std::endl;
//            return;
//        }
//    }
//    else {
//        std::cerr << "File does not exist: " << outputFilePath << std::endl;
//        return;
//    }
//}
//
//// Function to convert .doc, .docx, or .odt to .txt using LibreOffice's soffice command
//std::string convertToTxt(const std::string& inputFilePath, const std::string& outputDir) {
//
//    // Convert input file path to absolute path
//    std::filesystem::path inputAbsPath = std::filesystem::absolute(inputFilePath);
//
//    // Convert outputDir to a std::filesystem::path
//    std::filesystem::path outputDirPath(outputDir);
//
//    // Extract the file name without extension
//    std::filesystem::path inputPath(inputAbsPath);
//    std::string outputFileName = inputPath.stem().string() + ".txt";
//
//    // Combine the output directory and the output file name
//    std::filesystem::path outputFilePath = outputDirPath / outputFileName;
//
//    // Construct the Pandoc command with --wrap=none
//    std::string command = "pandoc --to=plain+smart --wrap=none \"" + inputAbsPath.string() + "\" -o \"" + outputFilePath.string() + "\"";
//
//    // Execute the command
//    int result = system(command.c_str());
//
//    // Check for errors during conversion
//    if (result != 0) {
//        std::cerr << "Error converting file: " << inputAbsPath.string() << std::endl;
//        throw std::runtime_error("Failed to convert file: " + inputAbsPath.string());
//    }
//    else {
//        std::cout << "File successfully converted to: " << outputFilePath.string() << std::endl;
//    }
//
//    // Temporary file for cleaned content
//    const std::string tempFilePath = "temp_output.txt";
//
//    // Open the original output file and the temporary file
//    std::ifstream inputFile(outputFilePath);
//    std::ofstream tempFile(tempFilePath);
//
//    if (!inputFile.is_open() || !tempFile.is_open()) {
//        std::cerr << "Error: Unable to open files for processing." << std::endl;
//        throw std::runtime_error("File not found: " + outputFilePath.string() + " or " + tempFilePath);
//    }
//
//    // Remove empty lines from the file
//    std::string line;
//    while (std::getline(inputFile, line)) {
//        if (!line.empty()) {  // Only write non-empty lines
//            tempFile << line << "\n";
//        }
//    }
//
//    // Close the files
//    inputFile.close();
//    tempFile.close();
//
//    deleteTemporaryFile(outputFilePath.string());
//
//    // Rename the temporary file to the original file name
//    try {
//        std::filesystem::rename(tempFilePath, outputFilePath);
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Error renaming temporary file: " << ex.what() << std::endl;
//    }
//
//    // Return the output file path as a string
//    return outputFilePath.string();
//
//}

// Represents a file in the system
class File {

    std::string name;
    std::string path;
    std::string format;

public:
    File(const std::string& path) {
        this->path = path;
        this->name = std::filesystem::path(path).filename().string();
        this->format = std::filesystem::path(path).extension().string();
    }

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

    // Function to convert .docx, or .odt to .txt using Pandoc's command
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

    /*std::vector<std::string> retrieveContent() {
        std::ifstream file(path);
        if (!file)
            throw std::runtime_error("File not found: " + path);

        // FR.11
        if (file.peek() == std::ifstream::traits_type::eof()) {
            throw std::runtime_error("Error: The file \"" + path + "\" is empty.");
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            // Trim whitespace and check if the line is empty
            if (!line.empty()) {
                lines.push_back(line);
            }
        }

        file.close();
        return lines;
    }*/

    std::string getPath() { return path; }
    std::string getName() { return name; }
    std::string getExtension() { return format; }

};

// Represents a difference between two files
class Difference {

    int lineNumber;
    std::string firstFileContent;
    std::string secondFileContent;

public:
    Difference(int lineNumber, const std::string& firstFileContent, const std::string& secondFileContent) {
        this->lineNumber = lineNumber;
        this->firstFileContent = firstFileContent;
        this->secondFileContent = secondFileContent;
    }      

    // Methods used to show the list of differences in terms of line number and the corresponding content
    int getLineNumber() const { return lineNumber; }
    const std::string& getFirstFileContent() const { return firstFileContent; }
    const std::string& getSecondFileContent() const { return secondFileContent; }
};

// Responsible for comparing two input files
class Comparator {

    std::vector<Difference> differences;

public:

    void compareFilesContent(const std::string file1Path, const std::string file2Path, std::string& file1Str, std::string& file2Str) {

        // Open files
        std::ifstream file1(file1Path);
        std::ifstream file2(file2Path);
        if (!file1) throw std::runtime_error("File not found: " + file1Path);
        if (!file2) throw std::runtime_error("File not found: " + file2Path);

        //Prepare content buffers and differences
        std::ostringstream file1Content, file2Content;
        differences.clear();
        std::string line1, line2;
        int lineNumber = 0;

        while (true)
        {
            //std::string line1, line2;
            bool gotLine1 = static_cast<bool>(std::getline(file1, line1));
            bool gotLine2 = static_cast<bool>(std::getline(file2, line2));
            lineNumber++;

            if (!gotLine1 && !gotLine2)
            {
                break;
            }

            std::string currentLine1 = gotLine1 ? line1 : "";
            std::string currentLine2 = gotLine2 ? line2 : "";

            //Compare and store
            if (currentLine1 != currentLine2) {
                differences.emplace_back(lineNumber, currentLine1, currentLine2);
            }

            if (gotLine1) file1Content << line1 << "\n";
            if (gotLine2) file2Content << line2 << "\n";
        }

        // Close the files
        file1.close();
        file2.close();

        file1Str = file1Content.str();
        file2Str = file2Content.str();

    }

    std::vector<Difference> getDifferences() { return differences; }
};

// Handles merging of two files
//class Merger {
//
//public:
//    std::vector<std::string> mergeFiles(const std::vector<std::string>& firstFileContent, 
//        const std::vector<std::string>& secondFileContent, 
//        const std::vector<Difference>& differences) {
//        std::vector<std::string> mergedContent;
//
//        //size_t maxLines = std::max(firstFileContent.size(), secondFileContent.size());
//        size_t diffIndex = 0;
//
//        for (size_t i = 0; i < maxLines; i++) {
//            if (diffIndex < differences.size() && i + 1 == differences[diffIndex].getLineNumber()) {
//                std::cout << "Line " << (i + 1) << " differs:\n";
//                std::cout << "1: " << differences[diffIndex].getFirstFileContent() << "\n";
//                std::cout << "2: " << differences[diffIndex].getSecondFileContent() << "\n";
//                std::cout << "Choose which line to save (1/2): ";
//
//                // FR.5
//                std::string choice;
//                std::cin >> choice;
//                while (choice != "1" && choice != "2") {
//                    std::cout << "Invalid choice. Please enter 1 or 2: ";
//                    std::cin >> choice;
//                }
//
//                mergedContent.push_back(choice == "1" ? differences[diffIndex].getFirstFileContent() : differences[diffIndex].getSecondFileContent());
//                diffIndex++;
//            }
//            else {
//                // To be improved, should be based on the user's choice but not based on file size
//                std::string line = (firstFileContent.size() > secondFileContent.size()) ? firstFileContent[i] : secondFileContent[i];
//                mergedContent.push_back(line);
//            }
//        }
//        return mergedContent;
//    }
//};


// Represents the final merged file
//class OutputFile {
//
//    std::vector<std::string> content;
//
//public:
//    OutputFile(const std::vector<std::string>& content) : content(content) {}
//
//    void saveContent(const std::string& outputPath) {
//        std::ofstream outputFile(outputPath);
//        if (!outputFile)
//            throw std::runtime_error("Failed to save file: " + outputPath);
//        
//        for (const auto& line : content) {
//            outputFile << line << "\n";
//        }
//        outputFile.close();
//    }
//
//    // FR.9 
//    void fileModification(const std::string& path) {
//
//        // User defined modification
//        std::cout << std::endl;
//        std::cout << "Do you want to modify the output file? Enter \"Y\" if you want.\n";
//        std::string choice;
//        std::cin >> choice;
//        if (choice == "Y")
//            modifyFileContent(path);       
//        else
//            std::cout << "Program closed. Please check the output file accordingly.\n";
//    }
//
//    // FR.9
//    void modifyFileContent(const std::string& path) {
//
//        std::cout << "Enter the line number and the content you want to changed.\n";
//        std::cout << "Line number: ";
//        std::string lineNumber;
//        std::cin >> lineNumber;
//        std::cout << "Content to be changed: ";
//        std::string newContent;
//        std::cin >> newContent;
//        std::cout << std::endl;
//
//        std::ifstream inputFile(path);
//        if (!inputFile)
//            throw std::runtime_error("File not found: " + path);
//
//        // Read the file into a vector of strings
//        std::vector<std::string> lines;
//        std::string line;
//        size_t currentLine = 1; // Line numbers start from 1
//
//        while (std::getline(inputFile, line)) {
//            if (currentLine == std::stoull(lineNumber)) {
//                line = newContent; // Modify the specific line
//            }
//            lines.push_back(line);
//            currentLine++;
//        }
//        inputFile.close();
//
//        // If the specified line number is larger than the number of lines in the file
//        if (std::stoull(lineNumber) > lines.size()) {
//            std::cerr << "Error: The specified line number " << lineNumber << " exceeds the number of lines in the file.\n";
//            return; // fileModification(path) is possible for further changes after error popping out, should be implemented on GUI
//        }
//
//        // Write the modified lines back to the file
//        std::ofstream outputFile(path, std::ios::out); // Open the file for writing (truncate mode)
//        if (!outputFile)
//            throw std::runtime_error("Error: Unable to open file \"" + path + "\" for writing.");
//
//        for (const auto& modifiedLine : lines) {
//            outputFile << modifiedLine << "\n";
//        }
//        outputFile.close();
//
//        std::cout << "Output file changed and saved accordingly.\n";
//
//        fileModification(path);
//    }
//};


//// Main Program
//int main(int argc, char * argv[]) {
//
//    // Default files for input and output
//    std::string firstFileDefaultPath = "Original.txt";
//    std::string SecondFileDefaultPath = "Modified.txt";
//    std::string outputFileDefaultPath = "Output.txt";
//
//    // FR.7 & FR.8
//    std::cout << "Enter the path for the first .txt file, leave empty for default file: ";
//    std::string firstFilePath;
//    std::getline(std::cin, firstFilePath);
//    if (firstFilePath.empty()) 
//        firstFilePath = firstFileDefaultPath;
//
//    std::cout << "Enter the path for the second .txt file, leave empty for default file: ";
//    std::string secondFilePath;
//    std::getline(std::cin, secondFilePath);
//    if (secondFilePath.empty()) 
//        secondFilePath = SecondFileDefaultPath;
//
//    std::cout << "Enter the path for the output .txt file, leave empty for default file: ";
//    std::string outputFilePath;
//    std::getline(std::cin, outputFilePath);
//    if (outputFilePath.empty()) 
//        outputFilePath = outputFileDefaultPath;
//
//    try {
//        File firstFile(firstFilePath);
//        File secondFile(secondFilePath);
//
//        // FR.11
//        std::vector<std::string> firstFileContent = firstFile.retrieveContent();
//        std::vector<std::string> secondFileContent = secondFile.retrieveContent();
//
//        Comparator comparator;
//        std::vector<Difference> differences = comparator.compareFiles(firstFileContent, secondFileContent);
//
//        // FR.5
//        Merger merger;
//        std::vector<std::string> mergedContent = merger.mergeFiles(firstFileContent, secondFileContent, differences);
//
//        // FR.1
//        OutputFile outputFile(mergedContent);
//        outputFile.saveContent(outputFilePath);
//        std::cout << "Output saved to " << outputFilePath << "\n\n";
//
//        // FR.3
//        comparator.showDifferences();
//
//        // FR.9
//        outputFile.fileModification(outputFilePath);
//    }
//    catch (const std::exception& ex) {
//        std::cerr << ex.what() << "\n";
//    }
//
//    return 0;
//
//}
