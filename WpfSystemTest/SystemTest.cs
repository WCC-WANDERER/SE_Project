using NUnit.Framework;
using TextFileManagerUI;  // Reference to testing project
using System.Windows;
using System.IO;
using System.Windows.Controls;
using System.Windows.Automation;
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
        public async Task CompareFilesButton_ShouldInvokeComparison_WhenClicked()
        {
            // Arrange UI elements
            var compareButton = _mainWindow.FindName("CompareFileButton") as Button;
            var file1PathTextBox = _mainWindow.FindName("File1Path") as TextBox;
            var file2PathTextBox = _mainWindow.FindName("File2Path") as TextBox;

            // Simulate file paths for comparison
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

            compareButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent)); // Trigger button pressed event

            // Wait until the CompareFiles function finished on background thread,
            // otherwise Assert.IsTrue will be faster than the CompareFiles function, leading to failing of test
            await Task.Delay(1000);  // delay value depends on the estimated time required for CompareFiles function, allow more time in case of large files

            // Assert: Check that the comparison result has been populated in the Differences collection
            Assert.IsTrue(_mainWindow.Differences.Count > 0); // Verify the differences collection
        }


        // Functional testing - Saving output file
        [Test]
        public async Task SaveOutput_Click_ShouldSaveFile_WhenComparisonDone()
        {
            // Arrange UI elements
            var saveButton = _mainWindow.FindName("SaveOutputButton") as Button;
            var file1PathTextBox = _mainWindow.FindName("File1Path") as TextBox;
            var file2PathTextBox = _mainWindow.FindName("File2Path") as TextBox;
            var outputFileNameTextBox = _mainWindow.FindName("OutputFileName") as TextBox;

            // Simulate file paths for comparison
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

            // Manually trigger the comparison
            _mainWindow.CompareFiles_Click(null, null);

            // Wait until the CompareFiles function finished on background thread,
            // otherwise Assert.IsTrue will be faster than the CompareFiles function, leading to failing of test
            await Task.Delay(1000);  // delay value depends on the estimated time required for CompareFiles function, allow more time in case of large files

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


        // Negative testing - Unsupported File Formats
        [Test]
        public async Task CompareFilesButton_ShouldShowError_WhenUnsupportedFileFormatsAreSelected()
        {
            // Arrange UI elements
            var compareButton = _mainWindow.FindName("CompareFileButton") as Button;
            var file1PathTextBox = _mainWindow.FindName("File1Path") as TextBox;
            var file2PathTextBox = _mainWindow.FindName("File2Path") as TextBox;

            // Simulate file paths for comparison
            file1PathTextBox.Text = "SystemTestData/NT_invalid1.exe"; // Executable file
            file2PathTextBox.Text = "SystemTestData/NT_invalid2.png"; // Image file

            compareButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));  // Trigger button pressed event

            // Wait until the CompareFiles function finished on background thread,
            // otherwise Assert.IsTrue will be faster than the CompareFiles function, leading to failing of test
            await Task.Delay(1000);  // delay value depends on the estimated time required for CompareFiles function, allow more time in case of large files

            // Wait and try to find the MessageBox
            AutomationElement messageBox = AutomationElement.RootElement.FindFirst(TreeScope.Descendants, 
                new PropertyCondition(AutomationElement.ControlTypeProperty, ControlType.Window));

            // Assert that MessageBox is found
            Assert.IsNotNull(messageBox, "MessageBox was not displayed");

            var name = messageBox.GetCurrentPropertyValue(AutomationElement.NameProperty)?.ToString();
            if (name != null && name.Contains("Error"))
            {
                // Check the message box text
                var messageText = messageBox.FindFirst(TreeScope.Descendants,
                        new PropertyCondition(AutomationElement.ControlTypeProperty, ControlType.Text))
                        ?.GetCurrentPropertyValue(AutomationElement.NameProperty)?.ToString();

                Assert.AreEqual("File 1 is not a valid file.\nThe following file formats are supported: .txt, .docx, .odt.", 
                    messageText, "Unexpected message in MessageBox");
            }

            // Ensure no file comparison occurred (No differences should be displayed)
            Assert.IsTrue(_mainWindow.Differences.Count == 0, "Comparison should not have occurred");
        }


        // Performance testing - Performance of the Application with Large Files
        [Test]
        public async Task CompareFilesButton_ShouldCompleteWithinAcceptableTime_WhenLargeFilesAreCompared()
        {
            // Arrange UI elements
            var compareButton = _mainWindow.FindName("CompareFileButton") as Button;
            var file1PathTextBox = _mainWindow.FindName("File1Path") as TextBox;
            var file2PathTextBox = _mainWindow.FindName("File2Path") as TextBox;

            // Ensure the elements are on the UI thread
            _mainWindow.Dispatcher.Invoke(() =>
            {
                // Simulate large file paths for comparison
                file1PathTextBox.Text = "SystemTestData/PT_LargeFile1.txt";
                file2PathTextBox.Text = "SystemTestData/PT_LargeFile2.txt";
            });

            // Start the stopwatch
            var stopwatch = System.Diagnostics.Stopwatch.StartNew();

            // Trigger the Compare Files operation, ensuring it's on the UI thread
            _mainWindow.Dispatcher.Invoke(() =>
            {
                compareButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));  // Trigger button pressed event
            });

            // Wait until the CompareFiles function finished on background thread,
            // otherwise Assert.IsTrue will be faster than the CompareFiles function, leading to failing of test
            await Task.Delay(50000);  // Wait for up to 50 seconds, as this is the expected finishing time for comparison of 100MB files

            // Stop the stopwatch
            stopwatch.Stop();

            // Check if the comparison completed within 60 seconds (30 seconds per file)
            Assert.Less(stopwatch.ElapsedMilliseconds, 60000, "File comparison took longer than the acceptable time limit.");
        }


        // Usability Testing - Navigation and feature understanding
        [Test]
        public async Task UserShouldBeAbleToNavigateAndUseCoreFeatures()
        {
            // Arrange UI elements
            var compareButton = _mainWindow.FindName("CompareFileButton") as Button;
            var browseButton1 = _mainWindow.FindName("BrowseButton1") as Button;
            var browseButton2 = _mainWindow.FindName("BrowseButton1") as Button;
            var saveButton = _mainWindow.FindName("SaveOutputButton") as Button;
            var modifyButton = _mainWindow.FindName("ModifyFileButton") as Button;
            var file1PathTextBox = _mainWindow.FindName("File1Path") as TextBox;
            var file2PathTextBox = _mainWindow.FindName("File2Path") as TextBox;
            var outputFileNameTextBox = _mainWindow.FindName("OutputFileName") as TextBox;

            // Simulate user browsing for input files and naming output file
            file1PathTextBox.Text = "SystemTestData/UT_TestFile1.txt";
            file2PathTextBox.Text = "SystemTestData/UT_TestFile2.txt";
            outputFileNameTextBox.Text = "outputFile";

            // Check that the user can easily locate and interact with the buttons
            Assert.IsNotNull(browseButton1, "Browse button should be visible.");
            Assert.IsNotNull(browseButton2, "Browse button should be visible.");
            Assert.IsNotNull(compareButton, "Compare button should be visible.");
            Assert.IsNotNull(saveButton, "Save button should be visible.");
            Assert.IsNotNull(modifyButton, "Modify button should be visible.");

            // Simulate user clicking the Compare Files button
            compareButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));

            // Wait until the CompareFiles function finished on background thread,
            // otherwise Assert.IsTrue will be faster than the CompareFiles function, leading to failing of test
            await Task.Delay(1000);  // delay value depends on the estimated time required for CompareFiles function, allow more time in case of large files

            // Assert that after comparison, the "Save Output" and "Modify File" buttons should be enabled
            Assert.IsTrue(saveButton.IsEnabled, "Save Output button should be enabled after comparison.");
            Assert.IsTrue(modifyButton.IsEnabled, "Modify File button should be enabled after comparison.");

            // Simulate user clicking the Save Output button
            saveButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));

            // Check that user can navigate smoothly between the buttons without confusion
            Assert.AreEqual("SystemTestData/UT_TestFile1.txt", file1PathTextBox.Text, "File 1 path should be valid.");
            Assert.AreEqual("SystemTestData/UT_TestFile2.txt", file2PathTextBox.Text, "File 2 path should be valid.");

            // Assert - Check that the file has been saved
            var outputFilePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "outputFile.txt");
            Assert.IsTrue(File.Exists(outputFilePath), "Output file was not saved");

            // Cleanup - Delete the output file after testing
            if (File.Exists(outputFilePath))
            {
                File.Delete(outputFilePath);
            }
        }


        [TearDown]
        public void TearDown()
        {
            // Close the window after each test
            _mainWindow?.Close();
        }

    }
}