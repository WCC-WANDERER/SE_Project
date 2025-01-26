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


namespace TextFileManagerUI
{
    public partial class MainWindow : Window
    {

        // Import C++ DLL
        public const string TextFileManagerDLL = @"..\..\..\..\x64\Debug\TextFileManager.dll";

        [DllImport(TextFileManagerDLL, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        private static extern void RetrieveContent(string path, [Out] char[] buffer, int bufferSize);

        [DllImport(TextFileManagerDLL, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        private static extern int CompareFiles(string file1Path, string file2Path, [Out] char[] resultBuffer, int bufferSize);


        // ObservableCollection for binding the differences
        public ObservableCollection<LineDifference> Differences { get; set; }

        public MainWindow()
        {
            InitializeComponent();
            Differences = new ObservableCollection<LineDifference>();
            DifferencesGrid.ItemsSource = Differences; // Bind DataGrid to the Differences collection
        }

        // Event handler for Browse first file
        private void BrowseFile1_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog fileDialog = new OpenFileDialog
            {
                Filter = "Text Files|*.txt|Word Documents|*.doc|OpenDocument Text|*.odt",
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
                Filter = "Text Files|*.txt|Word Documents|*.doc|OpenDocument Text|*.odt",
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
        private void BrowseSaveLocation_Click(object sender, RoutedEventArgs e)
        {
            // Get the selected output format from the ComboBox
            string selectedFormat = OutputFormat.SelectedItem is ComboBoxItem selectedItem ? selectedItem.Content.ToString() : ".txt";

            // Set the correct filter based on the selected format
            string filter = selectedFormat switch
            {
                ".doc" => "Word Documents|*.doc",
                ".odt" => "OpenDocument Text|*.odt",
                _ => "Text Files|*.txt"
            };

            SaveFileDialog saveFileDialog = new SaveFileDialog
            {
                Filter = filter,
                Title = "Save Output File",
                DefaultExt = selectedFormat,
                FileName = OutputFileName.Text + selectedFormat // Set the default file name with the correct format
            };

            if (saveFileDialog.ShowDialog() == true)
            {
                // Set the file path to the selected save location
                SaveLocation.Text = saveFileDialog.FileName;

                // Update OutputFileName text with the name from the save file dialog (without the path)
                OutputFileName.Text = System.IO.Path.GetFileNameWithoutExtension(saveFileDialog.FileName);
            }
        }

        // Method to check if LibreOffice is installed
        private bool IsLibreOfficeInstalled()
        {
            // Check if `soffice` is available in PATH
            var processStartInfo = new ProcessStartInfo
            {
                FileName = "soffice",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            };

            try
            {
                using (var process = Process.Start(processStartInfo))
                {
                    process.WaitForExit();
                    return true;
                }
            }
            catch
            {
                // Check common LibreOffice installation directories
                string[] commonPaths = 
                {
                    @"C:\Program Files\LibreOffice\program\soffice.exe",
                    @"C:\Program Files (x86)\LibreOffice\program\soffice.exe",
                    @"/usr/bin/soffice",   // Linux
                    @"/usr/local/bin/soffice", // macOS
                };

                foreach (var path in commonPaths)
                {
                    if (File.Exists(path))
                        return true;
                }
            }

            // LibreOffice not found
            return false;
        }

        // Event handler for Compare Files
        private void CompareFiles_Click(object sender, RoutedEventArgs e)
        {
            string file1Path = File1Path.Text;
            string file2Path = File2Path.Text;

            // Check if files exist
            if (!File.Exists(file1Path) || !File.Exists(file2Path))
            {
                MessageBox.Show("One or both files not found.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            // Check for LibreOffice installation
            if (!IsLibreOfficeInstalled())
            {
                MessageBox.Show(
                    "LibreOffice is not installed on this system. Please install LibreOffice to enable file conversion from .doc or .odt to .txt.",
                    "Missing Dependency",
                    MessageBoxButton.OK,
                    MessageBoxImage.Warning);
                return;
            }

            try
            {
                // Allocate buffer for result
                char[] resultBuffer = new char[10240]; // Adjust size as needed

                // Call the DLL function
                int result = CompareFiles(file1Path, file2Path, resultBuffer, resultBuffer.Length);
                if (result == 0)
                {
                    string differences = new string(resultBuffer).TrimEnd('\0');
                    MessageBox.Show(differences, "File Differences", MessageBoxButton.OK, MessageBoxImage.Information);
                }
                else
                {
                    MessageBox.Show("Error occurred during file comparison.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }


            //// Clear previous differences
            //Differences.Clear();

            //// Get file paths
            //string file1Path = File1Path.Text;
            //string file2Path = File2Path.Text;

            //// Check if files exist
            //if (!File.Exists(file1Path) || !File.Exists(file2Path))
            //{
            //    MessageBox.Show("Both files must be selected and exist!", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            //    return;
            //}

            //// Read file contents line by line
            //string[] file1Lines = File.ReadAllLines(file1Path);
            //string[] file2Lines = File.ReadAllLines(file2Path);

            //// Determine the maximum number of lines
            //int maxLines = System.Math.Max(file1Lines.Length, file2Lines.Length);

            //// Compare files line by line
            //for (int i = 0; i < maxLines; i++)
            //{
            //    string file1Content = i < file1Lines.Length ? file1Lines[i] : "";
            //    string file2Content = i < file2Lines.Length ? file2Lines[i] : "";

            //    if (file1Content != file2Content)
            //    {
            //        Differences.Add(new LineDifference
            //        {
            //            LineNumber = i + 1,
            //            File1Content = file1Content,
            //            File2Content = file2Content,
            //            UseFile1 = true, // Default selection
            //            UseFile2 = false
            //        });
            //    }
            //}
        }

        // Event handler for Save Output
        private void SaveOutput_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(SaveLocation.Text))
            {
                MessageBox.Show("Please select a save location for the output file!", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            // Generate output based on user selections
            var outputLines = new ObservableCollection<string>();
            foreach (var diff in Differences)
            {
                outputLines.Add(diff.UseFile1 ? diff.File1Content : diff.File2Content);
            }

            // Save output to the selected file
            File.WriteAllLines(SaveLocation.Text, outputLines);
            MessageBox.Show("Output file saved successfully!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
        }
    }

    // Model for line differences
    public class LineDifference
    {
        public int LineNumber { get; set; }
        public string File1Content { get; set; }
        public string File2Content { get; set; }
        public bool UseFile1 { get; set; }
        public bool UseFile2 { get; set; }
    }




}
