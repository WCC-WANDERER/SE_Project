#include "pch.h"


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


    // Boundary testing - ...





}