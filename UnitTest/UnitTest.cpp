#include "pch.h"
#include <fstream>


namespace UnitTests {

    // Import the struct and function from C++ side
    extern "C" {

        struct FileComparisonResult {
            char* file1ReturnContent;
            char* file2ReturnContent;
            char* differences;
        };

        FileComparisonResult CompareFiles(const char* file1Path, const char* file2Path);
        void FreeMemory(char* ptr);

    }


    // Testing
    // Functional testing - Input files with identical content
    TEST(FileComparisonTests, IdenticalFiles_ShouldHaveEmptyDifferences) {

        // Initialize the test data
        const char* file1 = "UnitTestData/FT_IdenticalFile1.txt";
        const char* file2 = "UnitTestData/FT_IdenticalFile2.txt";

        // Call the DLL function
        FileComparisonResult result = CompareFiles(file1, file2);

        // Expect differences to be empty since files are identical
        EXPECT_STREQ(result.differences, "");

        // Free allocated memory manually using FreeMemory()
        FreeMemory(result.file1ReturnContent);
        FreeMemory(result.file2ReturnContent);
        FreeMemory(result.differences);
    }
    

    // Functional testing - Input files with different content
    TEST(FileComparisonTests, DifferentFiles_ShouldHaveDifferences) {

        // Initialize the test data
        const char* file1 = "UnitTestData/FT_DiffFile1.txt";
        const char* file2 = "UnitTestData/FT_DiffFile2.txt";

        // Call the DLL function
        FileComparisonResult result = CompareFiles(file1, file2);

        // Expect differences to be empty since files are identical
        EXPECT_STRNE(result.differences, "");

        // Free allocated memory manually using FreeMemory()
        FreeMemory(result.file1ReturnContent);
        FreeMemory(result.file2ReturnContent);
        FreeMemory(result.differences);
    }


    // Boundary testing - Compares empty file with a non-empty file
    TEST(FileComparisonTests, EmptyFileVsNonEmptyFile_ShouldDetectAllLinesAsNew) {

        // Initialize the test data
        const char* file1 = "UnitTestData/BT_EmptyFile.txt";  // Empty file (0 bytes)
        const char* file2 = "UnitTestData/BT_File2.txt";      // File with multiple lines

        // Call the DLL function
        FileComparisonResult result = CompareFiles(file1, file2);

        // Expect all lines in file2 to be detected as new/added
        EXPECT_STRNE(result.differences, "");  // Differences should not be empty

        // Count the number of lines in BT_File2.txt
        int expectedAddedLines = 0;
        std::ifstream file(file2);
        std::string line;
        while (std::getline(file, line)) {
            expectedAddedLines++;
        }
        file.close();

        // Count the number of lines in result.differences
        int actualAddedLines = 0;
        std::istringstream diffStream(result.differences);
        while (std::getline(diffStream, line)) {
            actualAddedLines++;
        }

        // Expect all lines in BT_File2.txt to be detected as new
        EXPECT_EQ(actualAddedLines, expectedAddedLines);

        // Free allocated memory
        FreeMemory(result.file1ReturnContent);
        FreeMemory(result.file2ReturnContent);
        FreeMemory(result.differences);
    }


}