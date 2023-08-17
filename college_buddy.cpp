#include <iostream> //all the required header file
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <numeric>

using namespace std;

// struct for djikstra algorithm

struct Node
{
    int id;
    double distance;
    Node(int i, double d) : id(i), distance(d) {}
    bool operator<(const Node &other) const
    {
        return distance > other.distance;
    }
};

// student class with all the required member variables

class Student
{
public:
    string name;
    string location;
    double rank;
    string course_choice;
    double fee;
    double latitude;
    double longitude;
};

// college class with all the required member variables

class College
{
public:
    string name;
    string location;
    double rank;
    string course;
    int max_students;
    double fee;
    double latitude;
    double longitude;
    College(string CollegeName, string CollegeLocation, double Collegerank, string Collegecourse, int seats, double fees, double lat, double longi)
    {
        name = CollegeName;
        location = CollegeLocation;
        rank = Collegerank;
        course = Collegecourse;
        max_students = seats;
        fee = fees;
        latitude = lat;
        longitude = longi;
    }
};

vector<College> college_list;
vector<string> college_name;

// globally declared vectors to get college name & details

void getcollegelist()
{

    // opening file
    ifstream infile("College.txt");

    // checking if file is accessible
    if (!infile)
    {
        cout << "Error: Could not open file." << endl;
    }

    string line;

    // loop reading the whole file and storing its data in vector
    while (getline(infile, line))
    {

        // split each line into its respective fields
        stringstream ss(line);
        string name, location, course;
        double latitude, longitude, fees;
        int max_seats;
        double cutoff;
        getline(ss, name, ',');
        getline(ss, location, ',');
        ss >> cutoff;
        ss.ignore();
        getline(ss, course, ',');
        ss >> max_seats;
        ss.ignore();
        ss >> fees;
        ss.ignore();
        ss >> latitude;
        ss.ignore();
        ss >> longitude;
        ss.ignore();

        // create a College object for the current line
        College college(name, location, cutoff, course, max_seats, fees, latitude, longitude);

        // store the college object
        college_list.push_back(college);

        // storing name of each college
        college_name.push_back(college.name);
    }

    cout << endl;

    // close the file
    infile.close();
}

// Returns a vector of pointers to suitable colleges for the given student
vector<College *> findCollegeRank(Student &student, vector<College> &colleges)
{
    // vector of pointers storing available college for student
    vector<College *> suitable_colleges;

    // vector of pointers storing available college for student on an general formula
    vector<College *> approximate_colleges;

    // sort colleges by percentile_cutoff
    sort(colleges.begin(), colleges.end(),
         [](const College &a, const College &b)
         {
             return a.rank < b.rank;
         });

    // binary search for first college with percentile_cutoff >= student percentile
    int first_index = 0;
    int last_index = colleges.size() - 1;
    int middle_index;
    while (first_index <= last_index)
    {
        middle_index = (first_index + last_index) / 2;
        if (colleges[middle_index].rank < student.rank)
        {
            first_index = middle_index + 1;
        }
        else
        {
            last_index = middle_index - 1;
        }
    }

    // iterate through remaining colleges to find suitable and approximate colleges
    for (int i = first_index; i < colleges.size(); i++)
    {
        auto &college = colleges[i];

        if (college.rank >= student.rank)
        {
            suitable_colleges.push_back(&college);
        }
        else if (college.rank >= student.rank - 5.0) // 5% approximation
        {
            approximate_colleges.push_back(&college);
        }
    }

    // lambda function which sorts using comparator
    sort(approximate_colleges.begin(), approximate_colleges.end(),
         [](College *a, College *b)
         {
             return a->rank > b->rank;
         });

    // finalized list of colleges
    suitable_colleges.insert(suitable_colleges.end(), approximate_colleges.begin(),
                             approximate_colleges.end());
    return suitable_colleges;
}

// function taking latitude and longitude of both college and student

double distance(double lat1, double lon1, double lat2, double lon2)
{
    // creating a single latitude and longitude point
    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    // great circle distance for calculating distance between student and college
    double a = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    const double R = 6371e3; // Earth's radius in meters
    return R * c;
}


//28.5355 , 77.3910
// finding the nearest college to the student using Dijkstra's algorithm
int findCollegeLocation(double student_lat, double student_lon, vector<College> &colleges)
{
    int n = colleges.size();

    // keep track of the final minimum distances from the starting node to all other nodes
    vector<double> distances(n, INFINITY);

    // vector for nodes which are visited
    vector<bool> visited(n, false);

    // priority queue to store each node index and their minimum distance
    priority_queue<Node> pq;
    pq.push(Node(0, 0));

    // while each node is not visited
    while (!pq.empty())
    {
        Node node = pq.top();
        pq.pop();
        if (visited[node.id])
            continue;
        visited[node.id] = true;
        distances[node.id] = node.distance;
        for (int i = 0; i < n; i++)
        {
            if (i == node.id)
                continue;
            // finding the great circle distance
            double dist = distance(colleges[node.id].latitude, colleges[node.id].longitude, colleges[i].latitude, colleges[i].longitude);
            if (distances[node.id] + dist < distances[i])
            {
                distances[i] = distances[node.id] + dist;
                pq.push(Node(i, distances[i]));
            }
        }
    }

    int nearest_college_id = -1;
    double min_distance = INFINITY;

    for (int i = 0; i < n; i++)
    {
        double dist = distance(student_lat, student_lon, colleges[i].latitude, colleges[i].longitude);
        if (dist < min_distance)
        {
            nearest_college_id = i;
            min_distance = dist;
        }
    }
    return nearest_college_id;
}

string stringMatching(string input)
{

    // Convert input to lowercase for case-insensitive search
    transform(input.begin(), input.end(), input.begin(), ::tolower);
    int i = 0, j = 0;
    // Search for the input in the list of colleges
    auto it = find_if(college_name.begin(), college_name.end(), [&](const string &find_name)
                      {
        string lowercase_college = find_name;
        
        transform(lowercase_college.begin(), lowercase_college.end(), lowercase_college.begin(), ::tolower);
        i=0,j=0;
        
        while (i < lowercase_college.length() && j < input.length()) {
            if (lowercase_college[i] == input[j]) {
                j++;
            }
            i++;
        }
        
        return j == input.length(); });

    int check;
    // If input is found, print the college name. Otherwise, print an error message.
    if (it != college_name.end())
    {
        cout << "Did you mean: " << *it << "?" << endl;
        cin >> check;
        if (check)
            return *it;
        else
            cout << "College not found." << endl;
    }
    else
    {
        cout << "College not found." << endl;
    }
    return "";
}



vector<string> getSentences(string paragraph)
{
    vector<string> sentences;
    int startPos = 0;
    for (int i = 0; i < paragraph.length(); i++)
    {
        if (paragraph[i] == '.' || paragraph[i] == '?' || paragraph[i] == '!')
        {
            sentences.push_back(paragraph.substr(startPos, i - startPos + 1));
            startPos = i + 1;
        }
    }
    return sentences;
}

vector<pair<string, int>> scoreSentences(vector<string> sentences)
{
    vector<pair<string, int>> scoredSentences;
    unordered_map<string, int> wordCounts;
    int maxCount = 0;

    for (string sentence : sentences)
    {
        int score = 0;
        for (char c : sentence)
        {
            if (isalpha(c))
            {
                wordCounts[string(1, tolower(c))]++;                         // Count the occurrence of each word in the sentence
                maxCount = max(maxCount, wordCounts[string(1, tolower(c))]); // Update the maximum count
            }
        }
        score = sentence.length() / 10 + maxCount; // Calculate the sentence score
        scoredSentences.push_back(make_pair(sentence, score));
    }

    return scoredSentences;
}

string getSummary(string paragraph, int summaryLength)
{
    vector<string> sentences = getSentences(paragraph);
    vector<pair<string, int>> scoredSentences = scoreSentences(sentences);

    // Sort the sentences in descending order of score
    sort(scoredSentences.begin(), scoredSentences.end(), [](const pair<string, int> &a, const pair<string, int> &b)
         { return a.second > b.second; });

    // Combine the top-scoring sentences up to the summary length
    int i = 0;
    string summary;
    while (i < scoredSentences.size() && summary.length() < summaryLength)
    {
        summary += scoredSentences[i].first;
        i++;
    }
    return summary;
}

vector<string> recommendStudyPlan(unordered_map<string, vector<string>> &subjectTopics,
                                  unordered_map<string, int> &topicFrequency,
                                  unordered_map<string, int> &studentPerformance,
                                  vector<string> &selectedSubjects)
{
    vector<string> studyPlan;

    // recommend study plan based on selected subjects and sorted topics
    for (auto &subject : selectedSubjects)
    {
        if (subjectTopics.count(subject))
        {
            // only recommend for subjects in the course
            auto &topics = subjectTopics[subject];

            // sort topics by frequency in decreasing order
            vector<pair<string, int>> sortedTopics;

            for (auto &topic : topics)
            {
                sortedTopics.emplace_back(topic, topicFrequency[topic]);
            }

            sort(sortedTopics.begin(), sortedTopics.end(), [](const auto &lhs, const auto &rhs)
                 { return lhs.second > rhs.second; });

            // recommend topics based on sorted topics and student performance
            for (auto it = sortedTopics.begin(); it != sortedTopics.end(); ++it)
            {
                auto &topic = it->first;
                if (studentPerformance[topic] < 5)
                {
                    studyPlan.push_back(topic);
                    studentPerformance[topic]++;
                }
            }
        }
    }

    return studyPlan;
}

// Function to get input from the user
void get_input(vector<pair<string, vector<int>>> &tasks)
{
    int n;
    cout << "Enter the number of tasks: ";
    cin >> n;

    // Getting the tasks and subtasks from the user
    for (int i = 0; i < n; i++)
    {
        string task_name;
        cout << "Enter the name of task " << i + 1 << ": ";
        cin >> task_name;

        int subtasks;
        cout << "Enter the number of subtasks for task " << i + 1 << ": ";
        cin >> subtasks;

        vector<int> subtask_times;
        cout << "Enter the time required for each subtask (in minutes): ";
        for (int j = 0; j < subtasks; j++)
        {
            int time;
            cin >> time;
            subtask_times.push_back(time);
        }

        tasks.push_back(make_pair(task_name, subtask_times));
    }
}

// Function to calculate the plan
void calculate_plan(vector<pair<string, vector<int>>> &tasks)
{
    vector<pair<string, int>> plan;
    int total_time = 0;

    // Calculate the total time required
    for (auto task : tasks)
    {
        total_time += accumulate(task.second.begin(), task.second.end(), 0);
    }

    // Calculate the number of breaks required
    int breaks = total_time / 60;

    // Sort tasks in descending order of the total time required for each task
    sort(tasks.begin(), tasks.end(), [](auto &left, auto &right)
         { return accumulate(left.second.begin(), left.second.end(), 0) > accumulate(right.second.begin(), right.second.end(), 0); });

    // Add tasks to the plan, taking breaks after each task
    for (auto task : tasks)
    {
        int task_time = accumulate(task.second.begin(), task.second.end(), 0);
        plan.push_back(make_pair(task.first, task_time));

        if (breaks > 0)
        {
            plan.push_back(make_pair("Break", 10));
            breaks--;
        }
    }

    // Print the plan
    cout << "Your plan for the day is:\n";
    for (auto task : plan)
    {
        cout << task.first << " - " << task.second << " minutes\n";
    }
}

// Print the names and fees of the colleges in the given vector
void printColleges(vector<College *> colleges)
{
    if (colleges.size() == 0)
    {
        cout << "No suitable colleges found." << endl;
        return;
    }
    cout << "Suitable colleges:" << endl
         << endl;
    for (auto college : colleges)
    {
        cout << college->name << " " << college->location << " " << college->rank << " " << college->course << " " << college->max_students << " " << college->fee << " " << college->latitude << " " << college->longitude << " " << endl;
    }
    cout << endl
         << "Total suitable colleges found: " << colleges.size() << endl; // added for debugging
}

int main()
{
    bool run;
    do
    {
        getcollegelist();
        cout << endl;

        cout << "                 ***************************************************************************************            " << endl;

        cout << "                                                 WELCOME TO COLLEGE BUDDY                                   " << endl
             << endl;

        cout << "                 ***************************************************************************************             " << endl;

        cout << "                             Set a specific goal for what you want to achieve during this study,\n                                whether it's seeking guidance for college, preparing for exams,\n                          summarizing text, or improving your focus and productivity by managing your time effectively." << endl
             << endl;

        cout << "                                          MAIN MENU :-       " << endl;

        cout << "                                          1. College Selector" << endl;

        cout << "                                          2. College Explorer" << endl;

        cout << "                                          3. Summarizer" << endl;

        cout << "                                          4. Exam Coach" << endl;

        cout << "                                          5. Time Planner" << endl;

        cout << "Enter your choice(1-5):";
        int choice;
        cin >> choice;

        if (choice == 1)
        {
            string student_name, student_location, student_course;
            double student_rank, student_fees, student_latitude, student_longitude;

            cout << "Enter your Details" << endl;
            cout << "Enter your name:";
            cin >> student_name;

            cout << "Enter your location:";
            cin >> student_location;

            cout << "Enter your course choice:";
            cin >> student_course;

            cout << "Enter your rank:";
            cin >> student_rank;

            cout << "Enter your fees choice:";
            cin >> student_fees;

            cout << "Enter your location latitude:";
            cin >> student_latitude;

            cout << "Enter your longitude:";
            cin >> student_longitude;

            cout << "Enter on what basis you want to select your college:-" << endl;
            cout << "1. JEE RANK" << endl
                 << "2. LOCATION" << endl
                 << endl;

            int preference;
            cin >> preference;
            cout << endl;

            Student student = {student_name, student_location, student_rank, student_course, student_fees, student_latitude, student_longitude};

            if (preference == 1)
            {
                vector<College *> suitable_colleges;
                suitable_colleges = findCollegeRank(student, college_list);
                printColleges(suitable_colleges);
            }

            else
            {
                int nearest_college_id = findCollegeLocation(student.latitude, student.longitude, college_list);
                if (nearest_college_id >= 0)
                {
                    cout << "Nearest college to student: " << college_list[nearest_college_id].name << endl;
                }
                else
                {
                    cout << "No colleges found" << endl;
                }
            }
        }

        else if (choice == 2)
        {

            // Ask user for input
            string input;
            cout << "Enter the name of a college: ";
            cin >> input;
            bool check = true;
            for (auto i : college_list)
            {
                //   cout<<i.name<<" "<<endl;
                if (input == i.name)
                {
                    cout << i.name << " " << i.location << " " << i.rank << " " << i.course << " " << i.max_students << " " << i.fee << " " << i.latitude << " " << i.longitude << " " << endl;
                    check = false;
                }
            }
            if (check)
            {
                string ans = stringMatching(input);
                for (auto i : college_list)
                {
                    if (ans == i.name)
                    {
                        cout << i.name << " " << i.location << " " << i.rank << " " << i.course << " " << i.max_students << " " << i.fee << " " << i.latitude << " " << i.longitude << " " << endl;
                    }
                }
            }
        }
        else if (choice == 3)
        {
            ifstream inputFile("summarize.txt"); // Open input file stream object
            string paragraph, line;

            // Read the entire file into a string
            string fileContents((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());

            // Find the start and end of the paragraph in the fileContents string
            size_t startPos = fileContents.find("Start of paragraph");
            size_t endPos = fileContents.find("End of paragraph");

            // Extract the paragraph from the fileContents string
            if (startPos != string::npos && endPos != string::npos)
            {
                paragraph = fileContents.substr(startPos, endPos - startPos);
            }

            inputFile.close();
            int summaryLength = 900;
            string summary = getSummary(paragraph, summaryLength);
            cout << paragraph << endl
                 << endl;
            cout << summary << endl;
        }
        else if (choice == 4)
        {
            // read subjectTopics and topicFrequency from file
            ifstream file("topics.txt");
            unordered_map<string, vector<string>> subjectTopics;
            unordered_map<string, int> topicFrequency;

            if (file.is_open())
            {
                string line;
                while (getline(file, line))
                {
                    stringstream ss(line);
                    string subject;
                    ss >> subject;
                    string topic;
                    while (ss >> topic)
                    {
                        subjectTopics[subject].push_back(topic);
                        topicFrequency[topic] = 0;
                    }
                }
                file.close();
            }
            else
            {
                cout << "Unable to open file" << endl;
                return 1;
            }

            // read topicFrequency from file
            file.open("topics_frequency.txt");
            if (file.is_open())
            {
                string line;
                while (getline(file, line))
                {
                    stringstream ss(line);
                    string topic;
                    int frequency;
                    ss >> topic >> frequency;
                    topicFrequency[topic] = frequency;
                }
                file.close();
            }
            else
            {
                cout << "Unable to open file" << endl;
                return 1;
            }

            // take studentPerformance data as input from user
            unordered_map<string, int> studentPerformance;
            vector<string> selectedSubjects;

            for (auto &subject : subjectTopics)
            {
                cout << "Do you have " << subject.first << " in your course? (y/n): ";
                char input;
                cin >> input;
                if (input == 'y')
                {
                    selectedSubjects.push_back(subject.first);
                    for (auto &topic : subject.second)
                    {
                        cout << "Enter performance score(1-10) for " << topic << " in " << subject.first << ": ";
                        int score;
                        cin >> score;
                        studentPerformance[topic] = score;
                    }
                }
            }

            // recommend study plan
            vector<string> studyPlan = recommendStudyPlan(subjectTopics, topicFrequency, studentPerformance, selectedSubjects);

            // print recommended study plan
            cout << "Recommended study plan:" << endl;
            for (auto &topic : studyPlan)
            {
                cout << "- " << topic << endl;
            }
        }
        else
        {
            vector<pair<string, vector<int>>> tasks;
            get_input(tasks);
            calculate_plan(tasks);
        }
        cout << "Do you want to continue?";
        cin >> run;

    } while (run);

    return 0;
}