#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include <windows.h>
#include <ctime>
#include <sstream>
using namespace std;

// Define Task structure with new fields
struct Task {
    string name;
    string deadline;  // Format: "YYYY-MM-DD HH:MM"
    int priority;     // 1 (Low), 2 (Medium), 3 (High)
    bool isCompleted;
    string category;  // New: Task category
    string notes;     // New: Task notes

    Task(const string& taskName, const string& taskDeadline, int taskPriority, const string& taskCategory, const string& taskNotes, bool isComp = false)
        : name(taskName), deadline(taskDeadline), priority(taskPriority), category(taskCategory), notes(taskNotes), isCompleted(isComp) {}
};

// Data structures
map<string, string> userCredentials;
map<string, string> securityQuestions;
map<string, vector<Task>> userPersonalTasks;
map<string, vector<Task>> userCollegeTasks;

// Utility Functions
void displayTimer(int t) {
    cout << "Timer: " << t << " seconds remaining" << endl;
    Sleep(t * 1000);  // Windows Sleep in milliseconds
    Beep(1000, 500);
    cout << "Time's up!" << endl;
}

void updatePasswordInFile(string username, string newPassword) {
    userCredentials[username] = newPassword;
    ofstream credentialsFile("credentials.txt");
    for (const auto& entry : userCredentials) {
        credentialsFile << entry.first << " " << entry.second << "\n";
    }
    credentialsFile.close();
}

string formatTime(time_t t) {
    char* timeStr = ctime(&t);
    if (timeStr) {
        size_t len = strlen(timeStr);
        if (len > 0 && timeStr[len - 1] == '\n') timeStr[len - 1] = '\0';
        return string(timeStr);
    }
    return "Invalid time";
}

bool isTaskOverdue(const string& deadline) {
    time_t now = time(nullptr);
    struct tm tmDeadline = {0};
    istringstream iss(deadline);
    string date, time;
    getline(iss, date, ' ');  // YYYY-MM-DD
    getline(iss, time);       // HH:MM
    sscanf(date.c_str(), "%d-%d-%d", &tmDeadline.tm_year, &tmDeadline.tm_mon, &tmDeadline.tm_mday);
    sscanf(time.c_str(), "%d:%d", &tmDeadline.tm_hour, &tmDeadline.tm_min);
    tmDeadline.tm_year -= 1900;  // Adjust for tm struct (years since 1900)
    tmDeadline.tm_mon -= 1;      // Adjust for tm struct (0-11)
    time_t deadlineTime = mktime(&tmDeadline);
    return difftime(now, deadlineTime) > 0;
}

// Save user data to files
void saveUserData() {
    ofstream credentialsFile("credentials.txt");
    for (const auto& entry : userCredentials) {
        credentialsFile << entry.first << " " << entry.second << "\n";
    }
    credentialsFile.close();

    ofstream securityQuestionsFile("security_questions.txt");
    for (const auto& entry : securityQuestions) {
        securityQuestionsFile << entry.first << " " << entry.second << "\n";
    }
    securityQuestionsFile.close();

    ofstream personalTasksFile("personal_tasks.txt");
    for (const auto& entry : userPersonalTasks) {
        for (const Task& task : entry.second) {
            personalTasksFile << entry.first << " \"" << task.name << "\" \"" << task.deadline << "\" " 
                              << task.priority << " " << task.isCompleted << " \"" << task.category << "\" \"" << task.notes << "\"\n";
        }
    }
    personalTasksFile.close();

    ofstream collegeTasksFile("college_tasks.txt");
    for (const auto& entry : userCollegeTasks) {
        for (const Task& task : entry.second) {
            collegeTasksFile << entry.first << " \"" << task.name << "\" \"" << task.deadline << "\" " 
                              << task.priority << " " << task.isCompleted << " \"" << task.category << "\" \"" << task.notes << "\"\n";
        }
    }
    collegeTasksFile.close();
}

// Load user data from files
void loadUserData() {
    ifstream credentialsFile("credentials.txt");
    string username, password, line;
    while (getline(credentialsFile, line)) {
        istringstream iss(line);
        if (iss >> username >> password) userCredentials[username] = password;
    }
    credentialsFile.close();

    ifstream securityQuestionsFile("security_questions.txt");
    string question;
    while (getline(securityQuestionsFile, line)) {
        istringstream iss(line);
        if (iss >> username) {
            getline(iss, question);
            question = question.substr(1);
            securityQuestions[username] = question;
        }
    }
    securityQuestionsFile.close();

    ifstream personalTasksFile("personal_tasks.txt");
    string taskUsername, taskName, taskDeadline, taskCategory, taskNotes;
    int taskPriority;
    bool isCompleted;
    while (getline(personalTasksFile, line)) {
        istringstream iss(line);
        if (iss >> taskUsername) {
            iss.ignore(1, '"');
            getline(iss, taskName, '"');
            iss.ignore(1, '"');
            getline(iss, taskDeadline, '"');
            iss >> taskPriority >> isCompleted;
            iss.ignore(1, '"');
            getline(iss, taskCategory, '"');
            iss.ignore(1, '"');
            getline(iss, taskNotes, '"');
            userPersonalTasks[taskUsername].emplace_back(taskName, taskDeadline, taskPriority, taskCategory, taskNotes, isCompleted);
        }
    }
    personalTasksFile.close();

    ifstream collegeTasksFile("college_tasks.txt");
    while (getline(collegeTasksFile, line)) {
        istringstream iss(line);
        if (iss >> taskUsername) {
            iss.ignore(1, '"');
            getline(iss, taskName, '"');
            iss.ignore(1, '"');
            getline(iss, taskDeadline, '"');
            iss >> taskPriority >> isCompleted;
            iss.ignore(1, '"');
            getline(iss, taskCategory, '"');
            iss.ignore(1, '"');
            getline(iss, taskNotes, '"');
            userCollegeTasks[taskUsername].emplace_back(taskName, taskDeadline, taskPriority, taskCategory, taskNotes, isCompleted);
        }
    }
    collegeTasksFile.close();
}

// Sort tasks by deadline
void sortTasksByDeadline(vector<Task>& tasks) {
    sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return a.deadline < b.deadline;
    });
}

// Calculate progress
double calculateProgress(const vector<Task>& tasks) {
    if (tasks.empty()) return 0.0;
    int completed = 0;
    for (const Task& task : tasks) {
        if (task.isCompleted) completed++;
    }
    return (double)completed / tasks.size() * 100.0;
}

// Search tasks
void searchTasks(const string& username, const string& query) {
    bool found = false;
    cout << "Search Results:\n";
    for (const auto& task : userPersonalTasks[username]) {
        if (task.name.find(query) != string::npos) {
            cout << "Personal - " << task.name << " | Deadline: " << task.deadline << " | Category: " << task.category << endl;
            found = true;
        }
    }
    for (const auto& task : userCollegeTasks[username]) {
        if (task.name.find(query) != string::npos) {
            cout << "College - " << task.name << " | Deadline: " << task.deadline << " | Category: " << task.category << endl;
            found = true;
        }
    }
    if (!found) cout << "No tasks found matching '" << query << "'.\n";
}

// Daily task summary
void dailyTaskSummary(const string& username) {
    time_t now = time(nullptr);
    cout << "Tasks Due Today or Soon (within 24 hours):\n";
    bool found = false;
    for (const auto& task : userPersonalTasks[username]) {
        struct tm tmDeadline = {0};
        istringstream iss(task.deadline);
        string date, time;
        getline(iss, date, ' ');
        getline(iss, time);
        sscanf(date.c_str(), "%d-%d-%d", &tmDeadline.tm_year, &tmDeadline.tm_mon, &tmDeadline.tm_mday);
        sscanf(time.c_str(), "%d:%d", &tmDeadline.tm_hour, &tmDeadline.tm_min);
        tmDeadline.tm_year -= 1900;
        tmDeadline.tm_mon -= 1;
        time_t deadlineTime = mktime(&tmDeadline);
        double hoursUntil = difftime(deadlineTime, now) / 3600.0;
        if (hoursUntil <= 24 && hoursUntil >= 0) {
            cout << "Personal - " << task.name << " | Deadline: " << task.deadline << " | " << hoursUntil << " hours left\n";
            found = true;
        }
    }
    for (const auto& task : userCollegeTasks[username]) {
        struct tm tmDeadline = {0};
        istringstream iss(task.deadline);
        string date, time;
        getline(iss, date, ' ');
        getline(iss, time);
        sscanf(date.c_str(), "%d-%d-%d", &tmDeadline.tm_year, &tmDeadline.tm_mon, &tmDeadline.tm_mday);
        sscanf(time.c_str(), "%d:%d", &tmDeadline.tm_hour, &tmDeadline.tm_min);
        tmDeadline.tm_year -= 1900;
        tmDeadline.tm_mon -= 1;
        time_t deadlineTime = mktime(&tmDeadline);
        double hoursUntil = difftime(deadlineTime, now) / 3600.0;
        if (hoursUntil <= 24 && hoursUntil >= 0) {
            cout << "College - " << task.name << " | Deadline: " << task.deadline << " | " << hoursUntil << " hours left\n";
            found = true;
        }
    }
    if (!found) cout << "No tasks due today or soon.\n";
}

int main() {
    loadUserData();
    int choice;
    string username, password, newPassword, answer;

    // Login/Register Loop
    while (true) {
        // cout << "Welcome to the Student Task Manager!" << endl;
        cout << "Welcome to TaskScholar!" << endl;
        cout << "1. Log in\n2. Register\n3. Quit\nEnter your choice: ";
        cin >> choice;

        if (choice == 1) {
            cout << "Enter your username: ";
            cin >> username;
            cout << "Enter your password: ";
            cin >> password;
            if (userCredentials.find(username) != userCredentials.end() && userCredentials[username] == password) {
                cout << "Login successful!\n";
                break;
            } else {
                cout << "Invalid credentials. Please try again.\n";
            }
        } else if (choice == 2) {
            cout << "Enter a new username: ";
            cin >> username;
            if (userCredentials.find(username) != userCredentials.end()) {
                cout << "Username already exists!\n";
                continue;
            }
            cout << "Enter a new password: ";
            cin >> password;
            userCredentials[username] = password;
            cout << "Set a security question (answer): ";
            cin.ignore();
            getline(cin, answer);
            securityQuestions[username] = answer;
            saveUserData();
            cout << "Sign up successful!\n";
            break;
        } else if (choice == 3) {
            return 0;
        } else {
            cout << "Invalid choice. Please try again.\n";
        }
    }

    // Main Menu
    while (true) {
        cout << "\nTaskScholar Main Menu\n1. Personal Tasks\n2. College Tasks\n3. View Progress\n4. Search Tasks\n5. Daily Summary\n6. Change Password\n7. Log Out\nEnter your choice: ";
        // cout << "\nMain Menu\n1. Personal Tasks\n2. College Tasks\n3. View Progress\n4. Search Tasks\n5. Daily Summary\n6. Change Password\n7. Log Out\nEnter your choice: ";
        cin >> choice;

        if (choice == 1 || choice == 2) {
            map<string, vector<Task>>& tasks = (choice == 1) ? userPersonalTasks : userCollegeTasks;
            string taskType = (choice == 1) ? "Personal" : "College";

            while (true) {
                cout << "\n" << taskType << " Tasks Menu\n1. Add Task\n2. View Tasks\n3. Mark Task as Completed\n4. Sort Tasks by Deadline\n5. Go Back\nEnter your choice: ";
                cin >> choice;

                if (choice == 1) {
                    string taskName, taskDeadline, taskCategory, taskNotes;
                    int taskPriority;
                    cout << "Enter task name: ";
                    cin.ignore();
                    getline(cin, taskName);
                    cout << "Enter task deadline (e.g., YYYY-MM-DD HH:MM): ";
                    getline(cin, taskDeadline);
                    cout << "Enter priority (1 = Low, 2 = Medium, 3 = High): ";
                    cin >> taskPriority;
                    if (taskPriority < 1 || taskPriority > 3) {
                        cout << "Invalid priority! Using Medium (2).\n";
                        taskPriority = 2;
                    }
                    cout << "Enter category (e.g., Homework, Project, Exam Prep): ";
                    cin.ignore();
                    getline(cin, taskCategory);
                    cout << "Enter notes (optional, press Enter to skip): ";
                    getline(cin, taskNotes);
                    tasks[username].emplace_back(taskName, taskDeadline, taskPriority, taskCategory, taskNotes);
                    saveUserData();
                    cout << "Task added successfully!\n";
                } else if (choice == 2) {
                    cout << taskType << " Tasks:\n";
                    if (tasks[username].empty()) {
                        cout << "No tasks.\n";
                    } else {
                        for (size_t i = 0; i < tasks[username].size(); i++) {
                            const Task& task = tasks[username][i];
                            string priorityStr = (task.priority == 1) ? "Low" : (task.priority == 2) ? "Medium" : "High";
                            string overdueStr = isTaskOverdue(task.deadline) && !task.isCompleted ? " [OVERDUE]" : "";
                            cout << i + 1 << ". " << task.name << " | Deadline: " << task.deadline 
                                 << " | Priority: " << priorityStr << " | Category: " << task.category 
                                 << " | Completed: " << (task.isCompleted ? "Yes" : "No") << overdueStr;
                            if (!task.notes.empty()) cout << " | Notes: " << task.notes;
                            cout << endl;
                        }
                    }
                } else if (choice == 3) {
                    if (tasks[username].empty()) {
                        cout << "No tasks to mark.\n";
                        continue;
                    }
                    for (size_t i = 0; i < tasks[username].size(); i++) {
                        const Task& task = tasks[username][i];
                        cout << i + 1 << ". " << task.name << " | " << task.deadline << " | " << (task.isCompleted ? "Yes" : "No") << endl;
                    }
                    cout << "Enter task number to mark as completed: ";
                    int taskNumber;
                    cin >> taskNumber;
                    if (taskNumber >= 1 && taskNumber <= (int)tasks[username].size()) {
                        tasks[username][taskNumber - 1].isCompleted = true;
                        saveUserData();
                        cout << "Task marked as completed!\n";
                    } else {
                        cout << "Invalid task number.\n";
                    }
                } else if (choice == 4) {
                    sortTasksByDeadline(tasks[username]);
                    saveUserData();
                    cout << "Tasks sorted by deadline!\n";
                } else if (choice == 5) {
                    break;
                } else {
                    cout << "Invalid choice. Please try again.\n";
                }
            }
        } else if (choice == 3) {
            double personalProgress = calculateProgress(userPersonalTasks[username]);
            double collegeProgress = calculateProgress(userCollegeTasks[username]);
            cout << "Progress Report:\n";
            cout << "Personal Tasks: " << personalProgress << "% completed\n";
            cout << "College Tasks: " << collegeProgress << "% completed\n";
        } else if (choice == 4) {
            string query;
            cout << "Enter search query: ";
            cin.ignore();
            getline(cin, query);
            searchTasks(username, query);
        } else if (choice == 5) {
            dailyTaskSummary(username);
        } else if (choice == 6) {
            cout << "Enter your current password: ";
            cin >> password;
            if (userCredentials[username] == password) {
                cout << "Enter your new password: ";
                cin >> newPassword;
                updatePasswordInFile(username, newPassword);
                cout << "Password changed successfully!\n";
            } else {
                cout << "Invalid password. Password change failed.\n";
            }
        } else if (choice == 7) {
            cout << "Logging out...\n";
            saveUserData();
            break;
        } else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
    return 0;
}