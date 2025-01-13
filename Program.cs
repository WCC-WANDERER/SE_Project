using System;
using System.Collections.Generic;
using System.IO;

// Represents a file in the system
class File
{
    private string Name;
    private string Path;
    private string Format;

    public File(string path)
    {
        Path = path;
        Name = System.IO.Path.GetFileName(path);
        Format = System.IO.Path.GetExtension(path);
    }

    public string[] RetrieveContent()
    {
        if (!System.IO.File.Exists(Path))
            throw new FileNotFoundException($"File not found: {Path}");
        return System.IO.File.ReadAllLines(Path);
    }
}

// Represents a difference between two files
class Difference
{
    private int LineNumber;
    private string File1Content;
    private string File2Content;

    public Difference(int lineNumber, string file1Content, string file2Content)
    {
        LineNumber = lineNumber;
        File1Content = file1Content;
        File2Content = file2Content;
    }

    public string getFile1() {  return File1Content; }
    public string getFile2() { return File2Content; }
    public int getLineN () { return LineNumber; }
}

// Responsible for comparing two input files
class Comparator
{
    public List<Difference> CompareFiles(string[] file1Lines, string[] file2Lines)
    {
        var differences = new List<Difference>();

       int maxLines = file1Lines.Length > file2Lines.Length 
            ? file1Lines.Length 
            : file2Lines.Length;

        for (int i = 0; i < maxLines; i++)
        {
            string line1 = i < file1Lines.Length 
                ? file1Lines[i] 
                : "";
            string line2 = i < file2Lines.Length 
                ? file2Lines[i] 
                : "";

            if (line1 != line2)
            {
                differences.Add(new Difference(i + 1, line1, line2));
            }
        }
        return differences;
    }
}

// Handles merging of two files
class Merger
{
    public OutputFile MergeFiles(File file1, File file2, List<Difference> differences)
    {
        string[] file1Lines = file1.RetrieveContent();
        string[] file2Lines = file2.RetrieveContent();
        var mergedContent = new List<string>();

        int maxLines = file1Lines.Length > file2Lines.Length 
            ? file1Lines.Length 
            : file2Lines.Length;

        int diffIndex = 0;

        for (int i = 0; i < maxLines; i++)
        {
            if (diffIndex < differences.Count) { 

            Console.WriteLine($"Line {i + 1} differs:");
            Console.WriteLine($"1: {differences[diffIndex].getFile1()}");
            Console.WriteLine($"2: {differences[diffIndex].getFile2()}");
            Console.Write("Choose which line to save (1/2): ");

            string choice = Console.ReadLine();
            while (choice != "1" && choice != "2")
            {
                Console.Write("Invalid choice. Please enter 1 or 2: ");
                choice = Console.ReadLine();
            }

            mergedContent.Add(choice == "1"
                ? differences[diffIndex].getFile1()
                : differences[diffIndex].getFile2());
            diffIndex++;
            }
        }

        return new OutputFile(mergedContent.ToArray());
    }
}

// Represents the final merged file
class OutputFile 
{
    private string[] Content;

    public OutputFile(string[] content)
    {
        Content = content;
    }

    public void Save(string outputPath)
    {
        System.IO.File.WriteAllLines(outputPath, Content);
    }
}

// Main Program
class Program
{
    static void Main(string[] args)
    {
        string path1 = "Modified.txt";
        string path2 = "Original.txt";
        string path3 = "Output.txt";

        Console.WriteLine("Enter the path for the first .txt file, leave empty for defoult file:");
        string filePath1 = Console.ReadLine();
        if (filePath1 == "") { filePath1 = path1; }

        Console.WriteLine("Enter the path for the second .txt file, leave empty for defoult file:");
        string filePath2 = Console.ReadLine();
        if (filePath2 == "") { filePath2 = path2; }

        Console.WriteLine("Enter the path for the output .txt file, leave empty for defoult file:");
        string outputPath = Console.ReadLine();
        if (outputPath == "") { outputPath = path3; }

        try
        {
            var file1 = new File(filePath1);
            var file2 = new File(filePath2);

            string[] file1Lines = file1.RetrieveContent();
            string[] file2Lines = file2.RetrieveContent();

            var comparator = new Comparator();
            List<Difference> differences = comparator.CompareFiles(file1Lines, file2Lines);

            var merger = new Merger();
            OutputFile mergedFile = merger.MergeFiles(file1, file2, differences);

            mergedFile.Save(outputPath);

            Console.WriteLine($"Output saved to {outputPath}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"An error occurred: {ex.Message}");
        }
    }
}

