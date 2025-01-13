#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>


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

    std::vector<std::string> retrieveContent() {
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
            lines.push_back(line);
        }
        file.close();
        return lines;
    }
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
    std::vector<Difference> compareFiles(const std::vector<std::string>& firstFileContent, const std::vector<std::string>& secondFileContent) {
        //std::vector<Difference> differences;
        size_t maxLines = std::max(firstFileContent.size(), secondFileContent.size());

        for (size_t i = 0; i < maxLines; i++) {
            std::string firstFileLine = i < firstFileContent.size() ? firstFileContent[i] : "";
            std::string secondFileLine = i < secondFileContent.size() ? secondFileContent[i] : "";

            if (firstFileLine != secondFileLine) {
                differences.emplace_back(i + 1, firstFileLine, secondFileLine);
            }
        }
        return differences;
    }

    // FR.3; Temporary one for console application
    void showDifferences() { 

        // Title
        std::cout << std::endl;
        std::cout << "List of differences\n" << std::endl;

        // Header
        std::cout << std::left << std::setw(10) << "Line"
            << std::setw(70) << "First file Content"
            << std::setw(70) << "Second file Content" << std::endl;

        // Separator
        std::cout << std::string(150, '-') << std::endl;

        // Print each difference
        for (const auto& diff : differences) {
            std::cout << std::left << std::setw(10) << diff.getLineNumber()
                << std::setw(70) << diff.getFirstFileContent()
                << std::setw(70) << diff.getSecondFileContent() << std::endl;
        }
        std::cout << std::endl;
    }

    // GUI - To be changed to return vector of strings if necessary
    //std::vector<Difference> showDifferences() {  return differences;  }

};

// Handles merging of two files
class Merger {

public:
    std::vector<std::string> mergeFiles(const std::vector<std::string>& firstFileContent, 
        const std::vector<std::string>& secondFileContent, 
        const std::vector<Difference>& differences) {
        std::vector<std::string> mergedContent;

        size_t maxLines = std::max(firstFileContent.size(), secondFileContent.size());
        size_t diffIndex = 0;

        for (size_t i = 0; i < maxLines; i++) {
            if (diffIndex < differences.size() && i + 1 == differences[diffIndex].getLineNumber()) {
                std::cout << "Line " << (i + 1) << " differs:\n";
                std::cout << "1: " << differences[diffIndex].getFirstFileContent() << "\n";
                std::cout << "2: " << differences[diffIndex].getSecondFileContent() << "\n";
                std::cout << "Choose which line to save (1/2): ";

                // FR.5
                std::string choice;
                std::cin >> choice;
                while (choice != "1" && choice != "2") {
                    std::cout << "Invalid choice. Please enter 1 or 2: ";
                    std::cin >> choice;
                }

                mergedContent.push_back(choice == "1" ? differences[diffIndex].getFirstFileContent() : differences[diffIndex].getSecondFileContent());
                diffIndex++;
            }
            else {
                // To be improved, should be based on the user's choice but not based on file size
                std::string line = (firstFileContent.size() > secondFileContent.size()) ? firstFileContent[i] : secondFileContent[i];
                mergedContent.push_back(line);
            }
        }
        return mergedContent;
    }
};

//class Merger {
//public:
//    std::vector<std::string> mergeFiles(const std::vector<std::string>& file1Lines,
//        const std::vector<std::string>& file2Lines,
//        const std::vector<Difference>& differences) {
//        std::vector<std::string> mergedContent = file1Lines;
//        size_t diffIndex = 0;
//
//        for (const auto& diff : differences) {
//            int lineNumber = diff.getLineNumber();
//            std::cout << "Line " << lineNumber << " differs:\n";
//            std::cout << "1: " << diff.getFile1Content() << "\n";
//            std::cout << "2: " << diff.getFile2Content() << "\n";
//            std::cout << "Choose which line to save (1/2): ";
//
//            std::string choice;
//            while (true) {
//                std::cin >> choice;
//                if (choice == "1" || choice == "2") {
//                    break;
//                }
//                std::cout << "Invalid choice. Please enter 1 or 2: ";
//            }
//
//            mergedContent[lineNumber - 1] = (choice == "1") ? diff.getFile1Content() : diff.getFile2Content();
//        }
//
//        return mergedContent;
//    }
//};


// Represents the final merged file
class OutputFile {

    std::vector<std::string> content;

public:
    OutputFile(const std::vector<std::string>& content) : content(content) {}

    void saveContent(const std::string& outputPath) {
        std::ofstream outputFile(outputPath);
        if (!outputFile)
            throw std::runtime_error("Failed to save file: " + outputPath);
        
        for (const auto& line : content) {
            outputFile << line << "\n";
        }
        outputFile.close();
    }

    // FR.9 
    void fileModification(const std::string& path) {

        // User defined modification
        std::cout << std::endl;
        std::cout << "Do you want to modify the output file? Enter \"Y\" if you want.\n";
        std::string choice;
        std::cin >> choice;
        if (choice == "Y")
            modifyFileContent(path);       
        else
            std::cout << "Program closed. Please check the output file accordingly.\n";
    }

    // FR.9
    void modifyFileContent(const std::string& path) {

        std::cout << "Enter the line number and the content you want to changed.\n";
        std::cout << "Line number: ";
        std::string lineNumber;
        std::cin >> lineNumber;
        std::cout << "Content to be changed: ";
        std::string newContent;
        std::cin >> newContent;
        std::cout << std::endl;

        std::ifstream inputFile(path);
        if (!inputFile)
            throw std::runtime_error("File not found: " + path);

        // Read the file into a vector of strings
        std::vector<std::string> lines;
        std::string line;
        size_t currentLine = 1; // Line numbers start from 1

        while (std::getline(inputFile, line)) {
            if (currentLine == std::stoull(lineNumber)) {
                line = newContent; // Modify the specific line
            }
            lines.push_back(line);
            currentLine++;
        }
        inputFile.close();

        // If the specified line number is larger than the number of lines in the file
        if (std::stoull(lineNumber) > lines.size()) {
            std::cerr << "Error: The specified line number " << lineNumber << " exceeds the number of lines in the file.\n";
            return; // fileModification(path) is possible for further changes after error popping out, should be implemented on GUI
        }

        // Write the modified lines back to the file
        std::ofstream outputFile(path, std::ios::out); // Open the file for writing (truncate mode)
        if (!outputFile)
            throw std::runtime_error("Error: Unable to open file \"" + path + "\" for writing.");

        for (const auto& modifiedLine : lines) {
            outputFile << modifiedLine << "\n";
        }
        outputFile.close();

        std::cout << "Output file changed and saved accordingly.\n";

        fileModification(path);
    }
};


// Main Program
int main(int argc, char * argv[]) {

    // Default files for input and output
    std::string firstFileDefaultPath = "Original.txt";
    std::string SecondFileDefaultPath = "Modified.txt";
    std::string outputFileDefaultPath = "Output.txt";

    // FR.7 & FR.8
    std::cout << "Enter the path for the first .txt file, leave empty for default file: ";
    std::string firstFilePath;
    std::getline(std::cin, firstFilePath);
    if (firstFilePath.empty()) 
        firstFilePath = firstFileDefaultPath;

    std::cout << "Enter the path for the second .txt file, leave empty for default file: ";
    std::string secondFilePath;
    std::getline(std::cin, secondFilePath);
    if (secondFilePath.empty()) 
        secondFilePath = SecondFileDefaultPath;

    std::cout << "Enter the path for the output .txt file, leave empty for default file: ";
    std::string outputFilePath;
    std::getline(std::cin, outputFilePath);
    if (outputFilePath.empty()) 
        outputFilePath = outputFileDefaultPath;

    try {
        File firstFile(firstFilePath);
        File secondFile(secondFilePath);

        // FR.11
        std::vector<std::string> firstFileContent = firstFile.retrieveContent();
        std::vector<std::string> secondFileContent = secondFile.retrieveContent();

        Comparator comparator;
        std::vector<Difference> differences = comparator.compareFiles(firstFileContent, secondFileContent);

        // FR.5
        Merger merger;
        std::vector<std::string> mergedContent = merger.mergeFiles(firstFileContent, secondFileContent, differences);

        // FR.1
        OutputFile outputFile(mergedContent);
        outputFile.saveContent(outputFilePath);
        std::cout << "Output saved to " << outputFilePath << "\n\n";

        // FR.3
        comparator.showDifferences();

        // FR.9
        outputFile.fileModification(outputFilePath);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
    }

    return 0;

}
