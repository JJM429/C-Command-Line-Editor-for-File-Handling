#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <direct.h>

// Define constants for different text colours
#define COLOUR_ERROR 12 // Red colour for errors
#define COLOUR_SUCCESS 10 // Green colour for successes
#define COLOUR_INFO 14 // Yellow colour for information
#define COLOUR_FOLDER 9 // Blue colour for folders
#define COLOUR_TEXT 11 // Cyan colour for text files
#define COLOUR_DEFAULT 7 // White colour for default text

#define TEMP_FILE "temp.txt" // temporary file for operations that require a temp file


HANDLE hConsole; // Global variable to store console handle to set text attributes

// Function to set console text colour
void setColour(int colour) {
    SetConsoleTextAttribute(hConsole, colour);
}

//Function to retrieve the directory of the main executable file
void getExecutableDirectory(char *path, size_t size) {

    char executablePath[MAX_PATH]; // Buffer to hold the full path
    GetModuleFileName(NULL, executablePath, MAX_PATH); // Gets the executable path
    
    // Locates the last backslash to separate the directory
     char *lastSlash = strrchr(executablePath, '\\');
    if (lastSlash != NULL) {
        size_t directoryLength = lastSlash - executablePath;
        strncpy(path, executablePath, directoryLength); // Copies the directory position
        path[directoryLength] = '\0'; // ensures string is null terminated, so it is treated as a string
    }

} 

// Function to ensure a filename ends with .txt
void appendTxtExtension(char *filename) {
    if (!strstr(filename, ".txt")) {
        strcat(filename, ".txt"); // adds .txt if not present
    }
}

// Function to check if a file exists
int fileExists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1; // Files exists
    }
    return 0; // File does not exist
}

// Function to count number of lines in a file
int countLines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    int lineCount = 0;
    char buffer[256]; // Buffer to hold each line
    while(fgets(buffer, sizeof(buffer), file)) {
        lineCount++;
    }
    fclose(file);
    return lineCount;
}

// Function to get the size of a file in bytes
long getFileSize(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    fseek(file, 0, SEEK_END); // Seeks to the end of the file
    long size = ftell(file); // Gets position, which represents the size of the file
    fclose(file);
    return size;
}

// function to log actions in the changelog
void logChange(const char *filename, const char *action) {
    
    char logPath[MAX_PATH]; // Buffer to store the log file path

    getExecutableDirectory(logPath, sizeof(logPath));

    strcat(logPath, "\\changelog.txt"); // appends the changelog file name to the directory

    FILE *logFile = fopen(logPath, "a");
    if(!logFile) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open changelog.\n");
        setColour(COLOUR_DEFAULT);
        return;
    }

    time_t now = time(NULL);
    struct tm *local = localtime(&now); // Gets the current time and formats it
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", local);

    int lines = fileExists(filename) ? countLines(filename) : 0; // Counts the number of lines in the file
    long size = fileExists(filename) ? getFileSize(filename) : 0; // Gets the file size

    fprintf(logFile, "%s | Action: %s | File: %s | Lines: %d | Size: %ld bytes\n", timeStr, action, filename, lines, size);
    fclose(logFile);
}  

// Function to create a new file
void createFile(const char *filename) {
    if(fileExists(filename)) {
        setColour(COLOUR_ERROR);
        printf("Error: File %s already exists.\n", filename);
        setColour(COLOUR_DEFAULT);
        return;
    }

    FILE *file = fopen(filename, "w");
    if (file) {
        setColour(COLOUR_SUCCESS);
        printf("File %s created successfully!\n", filename);
        setColour(COLOUR_DEFAULT);
        fclose(file);
        logChange(filename, "Created");
    }
    else {
        setColour(COLOUR_ERROR);
        printf("Error: Could not create file %s.\n", filename);
        setColour(COLOUR_DEFAULT);
    }
}

// Function to delete a file if it exists
void deleteFile(const char *filename) {
    if (!fileExists(filename)) {
        setColour(COLOUR_ERROR);
        printf("Error: File %s does not exist.\n", filename);
        setColour(COLOUR_DEFAULT);
        return;
    }

    if (remove(filename) == 0) {
        setColour(COLOUR_SUCCESS);
        printf("File %s deleted successfully.\n", filename);
        setColour(COLOUR_DEFAULT);
        logChange(filename, "Deleted");
    }
    else {
        setColour(COLOUR_ERROR);
        printf("Error: Could not delete file %s.\n", filename);
        setColour(COLOUR_DEFAULT);
    }
}

// Function to copy a file
void copyFile(const char *source, const char *destination) {
    FILE *srcFile = fopen(source, "rb");
    if (!srcFile) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open source file %s.\n", source);
        setColour(COLOUR_DEFAULT);
        return;
    }

    FILE *destFile = fopen(destination, "wb");
    if (!destFile) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open destination file %s.\n", destination);
        setColour(COLOUR_DEFAULT);
        fclose(srcFile);
        return;
    }

    char buffer[1024]; // Buffer to transfer file content
    size_t bytesRead;
    while((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destFile);
    }

    fclose(srcFile);
    fclose(destFile);

    setColour(COLOUR_SUCCESS);
    printf("File %s copied to %s successfully.\n", source, destination);
    setColour(COLOUR_DEFAULT);

    logChange(destination, "Copied");
}

// Function to rename a file
void renameFile(const char *oldName, const char *newName) {
    if(!fileExists(oldName)) {
        setColour(COLOUR_ERROR);
        printf("Error: File %s does not exist.\n", oldName);
        setColour(COLOUR_DEFAULT);
        return;
    }

    if (fileExists(newName)) {
        setColour(COLOUR_ERROR);
        printf("Error: File %s already exists.\n", newName);
        setColour(COLOUR_DEFAULT);
        return;
    }

    if (rename(oldName, newName) == 0) {
        setColour(COLOUR_SUCCESS);
        printf("File %s renamed to %s successfully.\n", oldName, newName);
        setColour(COLOUR_DEFAULT);
        logChange(newName, "Renamed");
    }
    else {
        setColour(COLOUR_ERROR);
        printf("Error: Could not rename %s to %s.\n", oldName, newName);
        setColour(COLOUR_DEFAULT);
    }



}

// Function to display the contents of a file
void printFileContents(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open file %s.\n", filename);
        setColour(COLOUR_DEFAULT);
        return;
    }

    setColour(COLOUR_INFO);
    printf("Contents of %s:\n", filename);
    setColour(COLOUR_DEFAULT);

    char ch;
    while((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }
    printf("\n");
    fclose(file);
}

// Function to append a line to the end of a file
void appendLineToFile(const char *filename) {
    FILE *file = fopen(filename, "a"); // Opens the file in append mode
    if (!file) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open file %s.\n", filename);
        setColour(COLOUR_DEFAULT);
        return;
    }

    char line[256]; // Buffer the line to append
    getchar(); //clear input
    printf("Enter a line to append: ");
    fgets(line, sizeof(line), stdin); // Gets the input line
    line[strcspn(line, "\n")] = '\0'; // Removes the trailing new line character

    fprintf(file, "%s\n", line); // Writes the new line to the file
    fclose(file);

    setColour(COLOUR_SUCCESS);
    printf("Line appended successfully to %s.\n", filename);
    setColour(COLOUR_DEFAULT);

    logChange(filename, "Line Appended");
}

// Function to delete a specific line
void deleteLine(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open file %s.\n", filename);
        setColour(COLOUR_DEFAULT);
        return;
    }

    FILE *tempFile = fopen(TEMP_FILE, "w"); // creates a temp file
    if(!tempFile) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not create temporary file.\n");
        setColour(COLOUR_DEFAULT);
        fclose(file);
        return;
    }

    int deleteLineNumber, currentLine = 1; // line counters
    char buffer[256]; // buffer for file lines

    printf("Enter the line number to delete: ");
    scanf("%d", &deleteLineNumber);
    getchar();

    // Copies all lines except the target line to the temp file
    while (fgets(buffer, sizeof(buffer), file)) {
        if (currentLine != deleteLineNumber) {
            fputs(buffer, tempFile);
        }
        currentLine++;
    }

    fclose(file);
    fclose(tempFile);
    
    // Replace the original file with the temp file
    remove(filename);
    rename(TEMP_FILE, filename);

    setColour(COLOUR_SUCCESS);
    printf("Line %d deleted successfully from %s.\n", deleteLineNumber, filename);
    setColour(COLOUR_DEFAULT);

    logChange(filename, "Line Deleted");
}

// Function to insert a new line at a specific position in a file
void insertLine(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open file %s.\n", filename);
        setColour(COLOUR_DEFAULT);
        return;
    }

    FILE *tempFile = fopen(TEMP_FILE, "w");
    if (!tempFile) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not create temporary file.\n");
        setColour(COLOUR_DEFAULT);
        fclose(file);
        return;
    }

    int insertLineNumber, currentLine = 1;
    char buffer[256], newLine[256];

    printf("Enter the line number to insert at: ");
    scanf("%d", &insertLineNumber);
    getchar();
    printf("Enter the line to insert: ");
    fgets(newLine, sizeof(newLine), stdin);
    newLine[strcspn(newLine, "\n")] = '\0';

    while (fgets(buffer, sizeof(buffer), file)) {
        if (currentLine == insertLineNumber) {
            fprintf(tempFile, "%s\n", newLine);
        }
        fputs(buffer, tempFile);
        currentLine++;
    }

    if (insertLineNumber >= currentLine) {
        fprintf(tempFile, "%s\n", newLine);
    }

    fclose(file);
    fclose(tempFile);

    remove(filename);
    rename(TEMP_FILE, filename);
    
    setColour(COLOUR_SUCCESS);
    printf("Line inserted at line %d in %s successfully.\n", insertLineNumber, filename);
    setColour(COLOUR_DEFAULT);

    logChange(filename, "Line Inserted");
}

// Function to display a specific line from a file
void printLine(const char *filename) {
    FILE *file = fopen(filename, "r");
    if(!file) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open file %s.\n", filename);
        setColour(COLOUR_DEFAULT);
        return;
    }

    int lineNumber = 1, targetLine;
    char buffer[256];

    printf("Enter the line number to display: ");
    scanf("%d", &targetLine);

    if (targetLine < 1) {
        setColour(COLOUR_ERROR);
        printf("Invalid line number.\n");
        setColour(COLOUR_DEFAULT);
        fclose(file);
        return;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        if (lineNumber == targetLine) {
            setColour(COLOUR_INFO);
            printf("Line %d: %s", lineNumber, buffer);
            setColour(COLOUR_DEFAULT);
            fclose(file);
            return;
        }
        lineNumber++;
    }

    setColour(COLOUR_ERROR);
    printf("Error: Line %d does not exist. Total lines: %d.\n", targetLine, lineNumber - 1);
    setColour(COLOUR_DEFAULT);

    fclose(file);
}

// Function to list all files and directories in a given directory
void listDirectory(const char *path) {
    WIN32_FIND_DATA findFileData; // Structure that holds file information
    HANDLE hFind;

    char searchPath[256]; // Constructs a search path for the directory
    snprintf(searchPath, sizeof(searchPath), "%s\\*", path);

    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        setColour(COLOUR_ERROR);
        printf("Error: Could not open directory %S\n", path);
        setColour(COLOUR_DEFAULT);
        return;
    }

    setColour(COLOUR_INFO);
    printf("\nDirectory: %s\n", path);
    
    // Header for directory listing
    printf("------------------------------------------------------------------------------------------------------------------------------\n");
    printf("%-60s%-20s%-20s\n", "File Name", "Type", "Size (bytes)");
    printf("------------------------------------------------------------------------------------------------------------------------------\n");

    // Iterates through all files and subdirectories in the directory
    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) continue;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            setColour(COLOUR_FOLDER);
            printf("[Folder] %-60s%-20s%-20s\n", findFileData.cFileName, "Directory", "N/A");
        } else {
            setColour(COLOUR_DEFAULT);
            if (strstr(findFileData.cFileName, ".txt") != NULL) {
                setColour(COLOUR_TEXT);
            }
            printf("         %-60s%-20s%-20ld\n", findFileData.cFileName, "File", findFileData.nFileSizeLow);
            setColour(COLOUR_DEFAULT);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind); // Close the directory handle

    setColour(COLOUR_INFO);
    printf("------------------------------------------------------------------------------------------------------------------------------\n");
    setColour(COLOUR_DEFAULT);

}

// Function to provide a navigatable file explorer
void fileExplorer() {
    char currentPath[MAX_PATH]; // stores the current directory
    char input[256]; // input buffer

    // Print current working directory
    if (_getcwd(currentPath, sizeof(currentPath)) != NULL) {
        printf("Current Directory: %s\n", currentPath);
    }
    else {
        perror("getcwd() error");
        return;
    }

    while(1) {
        listDirectory(currentPath); // Displays the contents of the current directory

        printf("\nEnter a directory name to enter, '..' to go up or 'exit' to return: ");
        
        fgets(input, sizeof(input), stdin); // gets the users command

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            break; // exit the file explorer
        }

        if (strcmp(input, "..") == 0) {
            if (SetCurrentDirectory("..")) { // Navigates to the parent directory
                if (_getcwd(currentPath, sizeof(currentPath)) != NULL) {
                    printf("Current Directory: %s\n", currentPath); // Update and display
                } else {
                    perror("getcwd() error");
                    break;
                }
            } else {
                printf("Error: Could not go up to the parent directory.\n");
            }
            continue;
        }

        if (SetCurrentDirectory(input)) { // Attempt to change to given subdirectory
            if (_getcwd(currentPath, sizeof(currentPath)) != NULL) {
                printf("Current Directory: %s\n", currentPath); // update
            } else {
                perror("getcwd() error");
                break;
            }
        } else {
            printf("Error: Could not change directory to %s\n", input);
        }
    }
}

// Displays the help menu
void printHelp() {
    setColour(COLOUR_INFO);
    printf("\nHelp Menu:\n");
    setColour(COLOUR_DEFAULT);
    printf("This program has the following features:\n");
    printf("1. File Operations: Create, Copy, Delete, Rename, and View Files.\n");
    printf("2. Line Operations: Append, Delete, Insert, and View Lines.\n");
    printf("3. General Operations: View Changelog, Directory Listing, and Help.\n");
    printf("4. Directory Management: Navigate directories and list contents.\n");
}

// Function to handle user inpt and program navigation
void mainMenu() {
    int choice;
    char filename[MAX_PATH], newName[MAX_PATH], destination[MAX_PATH]; // file input buffers

    while (1) {
        setColour(COLOUR_INFO);
        printf("\nMain Menu:\n");
        setColour(COLOUR_DEFAULT);
        printf("1. File Operations\n");
        printf("2. Line Operations\n");
        printf("3. Directory Listing\n");
        printf("4. View Change Log\n");
        printf("5. Help Menu\n");
        printf("6. Quit\n");
        printf("Enter your Choice: ");
        scanf("%d", &choice); // gets the users choice

        // Clear the newline left by scanf
        while(getchar() != '\n');  

        switch (choice) {
            case 1: {
                int fileChoice;
                while(1) {
                    setColour(COLOUR_INFO);
                    printf("\nFile Operations:\n");
                    setColour(COLOUR_DEFAULT);
                    printf("1. Create File\n");
                    printf("2. Delete File\n");
                    printf("3. Copy File\n");
                    printf("4. Rename File\n");
                    printf("5. Show File Contents\n");
                    printf("6. Back to Main Menu\n");
                    printf("Enter your choice: ");
                    scanf("%d", &fileChoice);

                    // Clear the newline left by scanf
                    while(getchar() != '\n');

                    if (fileChoice == 6) break;

                    switch (fileChoice) {
                        case 1: //create
                        printf("Enter the name of the file to create: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        createFile(filename);
                        break;
                        
                        case 2: //delete
                        printf("Enter the name of the file to delete: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        deleteFile(filename);
                        break;

                        case 3: //copy
                        printf("Enter the name of the source file: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        printf("Enter the name of the destination file: ");
                        fgets(destination, sizeof(destination), stdin);
                        destination[strcspn(destination, "\n")] = 0;  
                        appendTxtExtension(destination);
                        copyFile(filename, destination);
                        break;

                        case 4: //rename
                        printf("Enter the current file name: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        printf("Enter the new file name: ");
                        fgets(newName, sizeof(newName), stdin);
                        newName[strcspn(newName, "\n")] = 0;  
                        appendTxtExtension(newName);
                        renameFile(filename, newName);
                        break;

                        case 5: //show contents
                        printf("Enter the name of the file to display: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        printFileContents(filename);
                        break;

                        default:
                        setColour(COLOUR_ERROR);
                        printf("Invalid Choice. Please try Again.\n");
                        setColour(COLOUR_DEFAULT);
                    }
                }
                break;
            }
            case 2: {
                int lineChoice;
                while(1) {
                    setColour(COLOUR_INFO);
                    printf("\nLine Operations:\n");
                    setColour(COLOUR_DEFAULT);
                    printf("1. Append Line\n");
                    printf("2. Delete Line\n");
                    printf("3. Insert Line\n");
                    printf("4. Show Specific Line\n");
                    printf("5. Count Lines in File\n");
                    printf("6. Back to Main Menu\n");
                    printf("Enter your choice: ");
                    scanf("%d", &lineChoice);

                    // Clear the newline left by scanf
                    while(getchar() != '\n');

                    if (lineChoice == 6) break;

                    switch (lineChoice) {
                        case 1: //append
                        printf("Enter the name of the file to append a line: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        appendLineToFile(filename);
                        break;
                        
                        case 2: //delete
                        printf("Enter the name of the file to delete a line: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        deleteLine(filename);
                        break;

                        case 3: //insert
                        printf("Enter the name of the file to insert a line: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        insertLine(filename);
                        break;

                        case 4: //Show specific line
                        printf("Enter the name of the file to show a specific line: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        printLine(filename);
                        break;

                        case 5: //count lines
                        printf("Enter the name of the file to count the number of lines: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = 0;  
                        appendTxtExtension(filename);
                        printf("Total Lines in %s: %d\n", filename, countLines(filename));
                        break;
                        
                        default:
                        setColour(COLOUR_ERROR);
                        printf("Invalid Choice.\n");
                        setColour(COLOUR_DEFAULT);
                    }
                }
                break;
            }
            case 3: // file explorer
            fileExplorer();
            break;

            case 4: // view changelog
            {
                char logPath[MAX_PATH];
                getExecutableDirectory(logPath, sizeof(logPath));
                strcat(logPath, "\\changelog.txt");
                
                printFileContents(logPath);
            }
            break;

            case 5: // show the help menu
            printHelp();
            break;

            case 6: // exit program
            setColour(COLOUR_SUCCESS);
            printf("Exiting program...\n");
            setColour(COLOUR_DEFAULT);
            return;

            default:
            setColour(COLOUR_ERROR);
            printf("Invalid Choice.\n");
            setColour(COLOUR_DEFAULT);
        }
    }
}

// Main function
int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // gets the console handle to set text attributes
    mainMenu(); // launch main menu
    return 0;
}

