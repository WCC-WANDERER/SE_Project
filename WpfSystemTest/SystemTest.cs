using NUnit.Framework;
using TextFileManagerUI;  // Reference to testing project
using System.Windows;
using System.IO;
using System.Windows.Controls;
using Microsoft.Win32;
using System.Diagnostics;
using Newtonsoft.Json.Linq;


namespace WpfSystemTests
{
    [TestFixture, Apartment(ApartmentState.STA)]  // Ensure using the STA thread for WPF
    public class SystemTests
    {
        private MainWindow _mainWindow;

        [SetUp]
        public void Setup()
        {
            // Run WPF application in the main thread (UI thread)
            _mainWindow = new MainWindow();
            _mainWindow.Show();
        }


        // Functional testing - Comparing input files
        [Test]
        public void CompareFilesButton_ShouldInvokeComparison_WhenClicked()
        {
            // Arrange
            var compareButton = _mainWindow.FindName("CompareFileButton") as Button;
            var file1PathTextBox = _mainWindow.FindName("File1Path") as TextBox;
            var file2PathTextBox = _mainWindow.FindName("File2Path") as TextBox;

            // Set file paths 
            file1PathTextBox.Text = "SystemTestData/FT_DiffFile1.odt";           // Testing supporting file format
            file2PathTextBox.Text = "SystemTestData/FT_DiffFile2.docx";          // Testing supporting file format

            // Ensure the files exist for comparison (text file only)
            if (!File.Exists(file1PathTextBox.Text) && System.IO.Path.GetExtension(file1PathTextBox.Text).ToLower() == ".txt")
            {
                File.WriteAllText(file1PathTextBox.Text, "File 1 content");
            }

            if (!File.Exists(file2PathTextBox.Text) && System.IO.Path.GetExtension(file2PathTextBox.Text).ToLower() == ".txt")
            {
                File.WriteAllText(file2PathTextBox.Text, "File 2 content");
            }

            compareButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));

            // Assert: Check that the comparison result has been populated in the Differences collection
            Assert.IsTrue(_mainWindow.Differences.Count > 0); // Verify the differences collection
        }

        // Functional testing - Saving output file
        [Test]
        public void SaveOutput_Click_ShouldSaveFile_WhenComparisonDone()
        {
            // Arrange
            var saveButton = _mainWindow.FindName("SaveOutputButton") as Button;
            var file1PathTextBox = _mainWindow.FindName("File1Path") as TextBox;
            var file2PathTextBox = _mainWindow.FindName("File2Path") as TextBox;
            var outputFileNameTextBox = _mainWindow.FindName("OutputFileName") as TextBox;

            // Simulate the file paths for comparison (assuming CompareFiles_Click has already been executed)
            file1PathTextBox.Text = "SystemTestData/FT_DiffFile1.txt";
            file2PathTextBox.Text = "SystemTestData/FT_DiffFile2.txt";
            outputFileNameTextBox.Text = "outputFile";

            // Ensure the files exist for comparison (text file only)
            if (!File.Exists(file1PathTextBox.Text) && System.IO.Path.GetExtension(file1PathTextBox.Text).ToLower() == ".txt")
            {
                File.WriteAllText(file1PathTextBox.Text, "File 1 content");
            }

            if (!File.Exists(file2PathTextBox.Text) && System.IO.Path.GetExtension(file2PathTextBox.Text).ToLower() == ".txt")
            {
                File.WriteAllText(file2PathTextBox.Text, "File 2 content");
            }

            // Manually trigger the comparison (this would be done earlier in your application)
            _mainWindow.CompareFiles_Click(null, null);

            // Act - Trigger the SaveOutput click
            saveButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));

            // Assert - Check that the file has been saved
            var outputFilePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "outputFile.txt");

            Assert.IsTrue(File.Exists(outputFilePath), "Output file was not saved");

            // Cleanup - Delete the output file after testing
            if (File.Exists(outputFilePath))
            {
                File.Delete(outputFilePath);
            }
        }


        // Negative testing - ...

        // Performance testing - ...

        // Usability testing - ...




        [TearDown]
        public void TearDown()
        {
            // Close the window after each test
            _mainWindow?.Close();
        }

    }
}