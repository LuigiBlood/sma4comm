using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Card_Collection_Compiler
{
    internal class Program
    {
        private static string renameOutput = "";
        static void generatePointerCode(string location, ref string classCode, string variableName)
        {
            string[] files = Directory.GetFiles(location, "*.bin", SearchOption.AllDirectories);
            bool[] needsOffset = new bool[files.Length];
            for (var i = 0; i < files.Length; i++)
            {
                var f = Path.GetFileNameWithoutExtension(files[i]);
                var safeF = ConvertFileNameToVariableSafe(f);
                if (f != safeF)
                {
                    var oldF = files[i];
                    Console.Write("Renaming " + files[i] + " to "); ;//Console.Write("Renaming " + files[i] + " to ");
                    files[i] = Path.GetDirectoryName(files[i]) + "/" + safeF + ".bin";
                    Console.WriteLine(files[i]);
                    File.Move(oldF, files[i]);


                }

            }
            foreach (string file in files)
            {
                classCode += "#include \"" + ConvertFileNameToVariableSafe(GetFolderAndFileFromPath(file)) + ".h\"\n";
            }
            classCode += "const u16* " + variableName + "Table[] = {";
            getTableNames(ref classCode, files, "(u16*)", "", true);
            classCode += "};\n";
            /*classCode += "const uint8_t* " + variableName + "Table_End[] = {";
            getTableNames(ref classCode, files, "&", "_end");
            classCode += "};\n";*/
            /*classCode += "static const size_t* " + variableName + "Table_Size[] = {";
            getTableNames(ref classCode, files, "&", "_size");
            classCode += "};\n";*/

            classCode += "static const char " + variableName + "Table_Names[" + files.Count() + "][64] = {";
            getTableNames(ref classCode, files, "\"", "\"");
            classCode += "};\n";
            classCode += "static const int " + variableName + "Table_Count = " + files.Count() + ";\n";

        }
        static void getTableNames(ref string classCode, string[] files, string prefix, string suffix, bool doOffsetCheck = false)
        {
            foreach (string file in files)
            {
                if (classCode[classCode.Length - 1] != '{')
                {
                    classCode += ", ";
                }
                var value = ConvertFileNameToVariableSafe(Path.GetFileNameWithoutExtension(file).Replace(" ", "_"), true) + "_bin" + suffix;

                if (doOffsetCheck)
                {
                    setOffsetCheck(ref value, file);
                }
                classCode += prefix + value;
            }
        }
        static void setOffsetCheck(ref string value, string file)//Check for the header, and offset the code if it's found.
        {
            string test = File.ReadAllText(file);
            if (test.Substring(0x1A, 8) == "NINTENDO")//For most common dumps.
            {
                value = "(" + value + "+0x72)";
            }
            if (test.Substring(0xC, 21) == "Super Mario Advance 4")//For the binaries produced by Smaghetti.
            {
                value = "(" + value + "+0x4E)";
            }
        }
        static string ConvertFileNameToVariableSafe(string filename, bool safeDigit = false)
        {
            if (safeDigit)//Used for the actual variable names.
            {
                string digits = "0123456789";
                if (digits.Contains(filename[0]))
                {
                    filename = "_" + filename;
                }
            }

            return filename.Replace("-", "_").Replace(".", "_").Replace(" ", "").Replace("'", "").Replace("(", "").Replace(")", "");
        }
        static int Main(string[] args)
        {
            Console.Write("Generating cardCollection.h ... ");
            renameOutput = "";
            //if (MessageBox.Show("This will rewrite the collection class to use the files currently present in the levels, demos, and powerups folders.\nIt will also rename certain binaries in the process if their name is incompatible.\nProceed?", "Confirm Compilation", MessageBoxButtons.YesNo) == DialogResult.Yes)
            //{
            string backoutCode = "../../../../";//Backout to root of source if called from this location.
            /*foreach (string arg in args)
            {
                Console.WriteLine(arg);
            }*/
            if (args.Length > 0 && args[0] == "-makefile")
            {
                backoutCode = "";
            }
            string classCode = "/*This is an automatically generated script by the Card Collection Compiler, used to map out all cards in the ROM. Use that tool to add your cards instead of manually adding them here.*/\n#pragma once\n";
            generatePointerCode(backoutCode + "powerups", ref classCode, "powerup");
            generatePointerCode(backoutCode + "levels", ref classCode, "level");
            generatePointerCode(backoutCode + "demos", ref classCode, "demo");
            /*classCode += "char* GetCardData(u16 mode, u16 index)"
            foreach (string file in files)
            {
                classCode += "void 



                    ";

            }*/

            
            File.WriteAllText("sample.c", classCode);
            if (File.Exists(backoutCode + "source/cardCollection.h"))
            {
                File.Delete(backoutCode + "source/cardCollection.h");
            }
            File.Move("sample.c", backoutCode + "source/cardCollection.h");
            Console.WriteLine("Done!");
            return 0;
            //}
        }
        static string GetFolderAndFileFromPath(string fullPath)
        {
            return Path.GetFileName(fullPath);
            /*
            fullPath = fullPath.Replace("\\", "/");
            // Split the path using forward slash as the directory separator
            string[] pathParts = fullPath.Split('/');

            // Extract the first two directories from the path, if they exist
            string folder = null;
            string file = null;
            if (pathParts.Length >= 2)
            {
                folder = pathParts[pathParts.Length - 2];
                file = pathParts[pathParts.Length - 1];
            }
            else if (pathParts.Length == 1)
            {
                // If the path has only one part, treat it as the file name
                file = pathParts[0];
            }

            // Return the folder and file parts, separated by a forward slash
            if (!string.IsNullOrEmpty(folder) && !string.IsNullOrEmpty(file))
            {
                return $"{folder}/{file}";
            }
            else if (!string.IsNullOrEmpty(file))
            {
                return file;
            }
            else
            {
                return string.Empty;
            }*/
        }
    }
}
