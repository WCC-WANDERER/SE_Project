using System.Collections.ObjectModel;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using System.Globalization;
using System.ComponentModel;


namespace TextFileManagerUI
{
    public partial class MainWindow : Window
    {

        // Import C++ DLL
        public const string TextFileManagerDLL = @"..\..\..\..\x64\Debug\TextFileManager.dll";

        [DllImport(TextFileManagerDLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void RetrieveContent(string path, [Out] char[] buffer, int bufferSize);

        [DllImport(TextFileManagerDLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int CompareFiles(string file1Path, string file2Path, [Out] char[] resultBuffer, int resultBufferSize, 
            [Out] char[] file1ContentBuffer, int file1ContentBufferSize, [Out] char[] file2ContentBuffer, int file2ContentbBufferSize);

        private bool isComparisonDone = false;
        private string firstFileContent;
        private string secondFileContent;
        private string lastSavedFilePath = string.Empty;  // Variable to store the path of the last saved file

        // ObservableCollection for binding the differences
        public ObservableCollection<LineDifference> Differences { get; set; }

        public MainWindow()
        {
            InitializeComponent();
            Differences = new ObservableCollection<LineDifference>();
            DifferencesGrid.ItemsSource = Differences; // Bind DataGrid to the Differences collection
            DifferencesGrid.DataContext = this; // Bind the DataContext to this window so the binding works
        }

        // Event handler for Browse first file
        private void BrowseFile1_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog fileDialog = new OpenFileDialog
            {
                Filter = "All Files|*.*|Text Files|*.txt|Word Documents|*.docx|OpenDocument Text|*.odt",
                Title = "Select your first file"
            };

            //string CombinedPath = System.IO.Path.Combine(Directory.GetCurrentDirectory(), "..", "..", "..", "..");
            //fileDialog.InitialDirectory = System.IO.Path.GetFullPath(CombinedPath);
            fileDialog.InitialDirectory = System.IO.Path.GetFullPath(Directory.GetCurrentDirectory());

            if (fileDialog.ShowDialog() == true)
            {
                //File1Path.Text = fileDialog.FileName;
                File1Path.Text = fileDialog.SafeFileName;
            }
            else
            {
                MessageBox.Show("File is not selected.");
            }
        }

        // Event handler for Browse second file
        private void BrowseFile2_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog fileDialog = new OpenFileDialog
            {
                Filter = "All Files|*.*|Text Files|*.txt|Word Documents|*.docx|OpenDocument Text|*.odt",
                Title = "Select your second file"
            };

            fileDialog.InitialDirectory = System.IO.Path.GetFullPath(Directory.GetCurrentDirectory());

            if (fileDialog.ShowDialog() == true)
            {
                File2Path.Text = fileDialog.SafeFileName;
            }
            else
            {
                MessageBox.Show("File is not selected.");
            }
        }

        // Event handler for Browse Save Location
        //private void BrowseSaveLocation_Click(object sender, RoutedEventArgs e)
        //{
        //    Get the selected output format from the ComboBox
        //    string selectedFormat = OutputFormat.SelectedItem is ComboBoxItem selectedItem ? selectedItem.Content.ToString() : ".txt";

        //    Set the correct filter based on the selected format
        //    string filter = selectedFormat switch
        //    {
        //        ".doc" => "Word Documents|*.doc",
        //        ".odt" => "OpenDocument Text|*.odt",
        //        _ => "Text Files|*.txt"
        //    };

        //    SaveFileDialog saveFileDialog = new SaveFileDialog
        //    {
        //        Filter = filter,
        //        Title = "Save Output File",
        //        DefaultExt = selectedFormat,
        //        FileName = OutputFileName.Text + selectedFormat // Set the default file name with the correct format
        //    };

        //    if (saveFileDialog.ShowDialog() == true)
        //    {
        //        Set the file path to the selected save location
        //        SaveLocation.Text = saveFileDialog.FileName;

        //        Update OutputFileName text with the name from the save file dialog(without the path)
        //        OutputFileName.Text = System.IO.Path.GetFileNameWithoutExtension(saveFileDialog.FileName);
        //    }
        //}

        // Method to check if LibreOffice is installed
        //private bool IsLibreOfficeInstalled()
        //{
        //    // Check if `soffice` is available in PATH
        //    var processStartInfo = new ProcessStartInfo
        //    {
        //        FileName = "soffice",
        //        UseShellExecute = false,
        //        RedirectStandardOutput = true,
        //        RedirectStandardError = true,
        //        CreateNoWindow = true
        //    };

        //    try
        //    {
        //        using (var process = Process.Start(processStartInfo))
        //        {
        //            process.WaitForExit();
        //            return true;
        //        }
        //    }
        //    catch
        //    {
        //        // Check common LibreOffice installation directories
        //        string[] commonPaths =
        //        {
        //            @"C:\Program Files\LibreOffice\program\soffice.exe",
        //            @"C:\Program Files (x86)\LibreOffice\program\soffice.exe",
        //            @"/usr/bin/soffice",   // Linux
        //            @"/usr/local/bin/soffice", // macOS
        //        };

        //        foreach (var path in commonPaths)
        //        {
        //            if (File.Exists(path))
        //                return true;
        //        }
        //    }

        //    // LibreOffice not found
        //    return false;
        //}

        // Method to check if Pandoc is installed
        private bool IsPandocInstalled()
        {
            // Check common Pandoc installation directories
            string[] commonPaths =
            {
                    @"C:\Users\cccsk\AppData\Local\Pandoc\pandoc.exe",  // Your own installation path
                    @"C:\Program Files\Pandoc\pandoc.exe",
                    @"C:\Program Files (x86)\Pandoc\pandoc.exe",
                    @"/usr/local/bin/pandoc",   // macOS and Linux
                    @"/usr/bin/pandoc", // Linux
            };

            foreach (var path in commonPaths)
            {
                if (File.Exists(path))
                    return true;
            }

            // Pandoc not found
            return false;
        }

        // Event handler for Compare Files
        private void CompareFiles_Click(object sender, RoutedEventArgs e)
        {
            // Initialize file path
            string file1Path = File1Path.Text;
            string file2Path = File2Path.Text;

            // Check if files exist
            if (!File.Exists(file1Path) || !File.Exists(file2Path))
            {
                MessageBox.Show("One or both files not found.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            // Check if either file is not a .txt file
            if (System.IO.Path.GetExtension(file1Path).ToLower() != ".txt" || System.IO.Path.GetExtension(file2Path).ToLower() != ".txt")
            {
                //Check for Pandoc installation
                if (!IsPandocInstalled())
                {
                    MessageBox.Show(
                        "Pandoc is not installed on this system. Please install Pandoc to enable file conversion from .docx or .odt to .txt.",
                        "Missing Dependency",
                        MessageBoxButton.OK,
                        MessageBoxImage.Warning);
                    return;
                }
            }

            try
            {
                // Allocate buffers for original content and differences
                char[] firstFileContentBuffer = new char[10240]; // Adjust size as needed
                char[] secondFileContentBuffer = new char[10240]; // Adjust size as needed
                char[] resultBuffer = new char[10240]; // For differences

                // Call the C++ function to compare files and get content and differences
                int result = CompareFiles(file1Path, file2Path, resultBuffer, resultBuffer.Length, 
                    firstFileContentBuffer, firstFileContentBuffer.Length, secondFileContentBuffer, secondFileContentBuffer.Length);
                if (result == 0)
                {
                    // Retrieve original content and differences from buffers
                    firstFileContent = new string(firstFileContentBuffer).TrimEnd('\0');
                    secondFileContent = new string(secondFileContentBuffer).TrimEnd('\0');

                    // Check if either file is empty
                    if (string.IsNullOrWhiteSpace(firstFileContent))
                    {
                        MessageBox.Show("File 1 is empty.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        return;
                    }

                    if (string.IsNullOrWhiteSpace(secondFileContent))
                    {
                        MessageBox.Show("File 2 is empty.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        return;
                    }

                    string differences = new string(resultBuffer).TrimEnd('\0');

                    // Parse the differences and update the ObservableCollection
                    Differences.Clear();
                    string[] diffLines = differences.Split('\n');
                    foreach (string line in diffLines)
                    {
                        if (!string.IsNullOrEmpty(line))
                        {
                            var parts = line.Split(new[] { ": File1 -> ", ", File2 -> " }, StringSplitOptions.None);
                            if (parts.Length == 3)
                            {
                                Differences.Add(new LineDifference
                                {
                                    LineNumber = int.Parse(parts[0].Replace("Line", "").Trim()),
                                    File1Content = parts[1].Trim(),
                                    File2Content = parts[2].Trim(),
                                    UseFile1 = true,  // Default selection
                                    UseFile2 = false
                                });
                            }
                        }
                    }

                    // Set the flag to true after comparison is done
                    isComparisonDone = true;

                    // Enable the "Save Output" button
                    SaveOutputButton.IsEnabled = true;  // Make sure SaveOutputButton is the correct reference to your button
                    ModifyFileButton.IsEnabled = true;

                    MessageBox.Show("Comparison completed successfully.", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
                }
                else
                {
                    MessageBox.Show("Error occurred during file comparison.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }

                // Check the extension of the files and delete the temporary .txt files if necessary
                if (System.IO.Path.GetExtension(file1Path).ToLower() != ".txt")
                {
                    // Remove extension from file1Path and add .txt
                    string tempFile1Path = System.IO.Path.GetFileNameWithoutExtension(file1Path) + ".txt";
                    if (File.Exists(tempFile1Path))
                    {
                        try
                        {
                            File.SetAttributes(tempFile1Path, FileAttributes.Normal);
                            File.Delete(tempFile1Path); // Delete the converted .txt file for file1
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show($"Error deleting temporary file for {file1Path}: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        }
                    }
                }
                if (System.IO.Path.GetExtension(file2Path).ToLower() != ".txt")
                {
                    // Remove extension from file2Path and add .txt
                    string tempFile2Path = System.IO.Path.GetFileNameWithoutExtension(file2Path) + ".txt";
                    if (File.Exists(tempFile2Path))
                    {
                        try
                        {
                            File.SetAttributes(tempFile2Path, FileAttributes.Normal);
                            File.Delete(tempFile2Path); // Delete the converted .txt file for file2
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show($"Error deleting temporary file for {file2Path}: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        // Event handler for Save Output
        private void SaveOutput_Click(object sender, RoutedEventArgs e)
        {
            if (!isComparisonDone)
            {
                MessageBox.Show("Please compare the files before saving the output.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            // Get the full content of the files (from C++ side)
            string file1Content = firstFileContent;  // This is the full content of file1
            string file2Content = secondFileContent;  // This is the full content of file2

            // Convert the content into lists of lines
            var file1Lines = file1Content.Split(new[] { '\n' }, StringSplitOptions.None).ToList();
            var file2Lines = file2Content.Split(new[] { '\n' }, StringSplitOptions.None).ToList();

            // Iterate over the Differences collection and update the content based on user choice
            foreach (var diff in Differences)
            {
                int lineNumber = diff.LineNumber - 1;  // Convert to 0-based index

                if (lineNumber >= 0 && lineNumber < file1Lines.Count)
                {
                    // Replace content in file1 or file2 based on user's choice
                    if (diff.UseFile2)  // If user chose File2 for this line
                    {
                        file1Lines[lineNumber] = diff.File2Content;  // Replace with content from File2
                    }
                    else if (diff.UseFile1)  // If user chose File1 for this line
                    {
                        file1Lines[lineNumber] = diff.File1Content;  // Keep content from File1
                    }
                }
            }

            // Filter out empty lines (lines that are either empty or contain only whitespace)
            var nonEmptyLines = file1Lines.Where(line => !string.IsNullOrWhiteSpace(line)).ToList();

            // Generate the output lines after merging (filtered to remove empty lines)
            var outputLines = new ObservableCollection<string>(nonEmptyLines);  // This will be your merged content

            // Select the format for saving
            string selectedFormat = OutputFormat.SelectedItem is ComboBoxItem selectedItem ? selectedItem.Content.ToString() : ".txt";

            // File dialog setup based on format selection
            string filter = selectedFormat switch
            {
                ".docx" => "Word Documents|*.docx",
                ".odt" => "OpenDocument Text|*.odt",
                _ => "Text Files|*.txt"
            };

            SaveFileDialog saveFileDialog = new SaveFileDialog
            {
                Filter = filter,
                Title = "Save Output File",
                DefaultExt = selectedFormat,
                FileName = OutputFileName.Text + selectedFormat // Default file name with the correct format
            };

            if (saveFileDialog.ShowDialog() == true)
            {
                try
                {
                    // Save the merged content to the selected file
                    string filePath = saveFileDialog.FileName;
                    File.WriteAllLines(filePath, outputLines);
                    lastSavedFilePath = filePath;  // Store the path of the saved file
                    MessageBox.Show("Output file saved successfully!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"An error occurred while saving the file: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        private void ModifyFile_Click(object sender, RoutedEventArgs e)
        {
            if (!isComparisonDone)
            {
                MessageBox.Show("Please compare the files before saving the output.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            if (string.IsNullOrEmpty(lastSavedFilePath))
            {
                MessageBox.Show("No file has been saved yet. Please save the output file first.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            try
            {
                // Open the file using the default application associated with the file type
                System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                {
                    FileName = lastSavedFilePath,
                    UseShellExecute = true  // Opens the file with its associated program
                });
            }
            catch (Exception ex)
            {
                MessageBox.Show($"An error occurred while opening the file: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        public class LineDifference : INotifyPropertyChanged
        {
            private bool useFile1;
            private bool useFile2;

            public int LineNumber { get; set; }
            public string File1Content { get; set; }
            public string File2Content { get; set; }

            public bool UseFile1
            {
                get { return useFile1; }
                set
                {
                    if (useFile1 != value)
                    {
                        useFile1 = value;
                        OnPropertyChanged(nameof(UseFile1));
                        //Debug.WriteLine($"UseFile1 changed to {value}"); // Debug testing

                        if (useFile1)
                        {
                            UseFile2 = false;  // If UseFile1 is true, set UseFile2 to false
                        }
                    }
                }
            }

            public bool UseFile2
            {
                get { return useFile2; }
                set
                {
                    if (useFile2 != value)
                    {
                        useFile2 = value;
                        OnPropertyChanged(nameof(UseFile2));
                        //Debug.WriteLine($"UseFile2 changed to {value}"); // Debug testing

                        if (useFile2)
                        {
                            UseFile1 = false;  // If UseFile2 is true, set UseFile1 to false
                        }
                    }
                }
            }

            // PropertyChanged event for data binding
            public event PropertyChangedEventHandler PropertyChanged;

            private void OnPropertyChanged(string propertyName)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }

}
