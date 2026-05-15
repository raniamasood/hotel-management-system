#include <iostream>
#include <string>

using namespace std;

/* ===================== HELPER FUNCTIONS ===================== */
string toUpper(string s)
{
    for (int i = 0; i < s.length(); i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
        {
            s[i] -= 32;
        }
    }
    return s;
}

bool validRoomType(string t)
{
    if (t == "SINGLE" || t == "DOUBLE" || t == "SUITE")
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* ===================== ROOM & FLOOR STRUCTURES ===================== */
struct Room
{
    int roomID;
    string type;
    string status; // Ready, Booked, Occupied, Unavailable
    Room* next;

    Room(int id, string t)
    {
        roomID = id;
        type = t;
        status = "Ready";
        next = NULL;
    }
};

struct Floor
{
    int floorNo;
    Room* roomHead;
    Floor* left;
    Floor* right;

    Floor(int f)
    {
        floorNo = f;
        roomHead = NULL;
        left = NULL;
        right = NULL;
    }
};

/* ===================== BOOKING QUEUE & HISTORY STACK ===================== */
struct BookingRequest
{
    string customer;
    string roomType;
    int nights;
    bool priority;
    BookingRequest* next;
};

struct BookingHistory
{
    string customer;
    int roomID;
    int nights;
    BookingHistory* next;
};

/* ===================== GLOBAL POINTERS ===================== */
Floor* root = NULL;
BookingRequest* highF = NULL;
BookingRequest* highR = NULL;
BookingRequest* regF = NULL;
BookingRequest* regR = NULL;
BookingHistory* historyTop = NULL;

/* ===================== FLOOR TREE FUNCTIONS ===================== */
Floor* insertFloor(Floor* node, int f)
{
    if (node == NULL)
    {
        return new Floor(f);
    }

    if (f < node->floorNo)
    {
        node->left = insertFloor(node->left, f);
    }
    else
    {
        node->right = insertFloor(node->right, f);
    }

    return node;
}

void addRoom(Floor* f, int id, string type)
{
    Room* r = new Room(id, type);
    r->next = f->roomHead;
    f->roomHead = r;
}

/* ===================== BOOKING FUNCTIONS ===================== */
void enqueueRequest(string name, string type, int nights, bool priority)
{
    BookingRequest* b = new BookingRequest;
    b->customer = name;
    b->roomType = type;
    b->nights = nights;
    b->priority = priority;
    b->next = NULL;

    if (priority == true)
    {
        if (highF == NULL)
        {
            highF = b;
            highR = b;
        }
        else
        {
            highR->next = b;
            highR = b;
        }
    }
    else
    {
        if (regF == NULL)
        {
            regF = b;
            regR = b;
        }
        else
        {
            regR->next = b;
            regR = b;
        }
    }
}

bool bookRoom(Floor* node, BookingRequest* req)
{
    if (node == NULL)
    {
        return false;
    }

    bool leftResult = bookRoom(node->left, req);
    if (leftResult == true)
    {
        return true;
    }

    Room* r = node->roomHead;
    while (r != NULL)
    {
        // Cannot book unavailable rooms
        if (r->type == req->roomType && r->status == "Ready")
        {
            r->status = "Booked";

            BookingHistory* h = new BookingHistory;
            h->customer = req->customer;
            h->roomID = r->roomID;
            h->nights = req->nights;
            h->next = historyTop;
            historyTop = h;

            cout << "\nBooking approved: " << req->customer
                 << " -> Room " << r->roomID
                 << " for " << req->nights << " nights" << endl;

            return true;
        }

        r = r->next;
    }

    return bookRoom(node->right, req);
}

void processBooking()
{
    BookingRequest* req = NULL;

    if (highF != NULL)
    {
        req = highF;
        highF = highF->next;
    }
    else if (regF != NULL)
    {
        req = regF;
        regF = regF->next;
    }
    else
    {
        cout << "\nNo pending booking requests.\n";
        return;
    }

    bool booked = bookRoom(root, req);

    if (booked == false)
    {
        cout << "\nBooking failed: No available room of requested type.\n";
    }

    delete req;
}

void checkIn(int roomID)
{
    Floor* f = root;
    while (f != NULL)
    {
        Room* r = f->roomHead;
        while (r != NULL)
        {
            if (r->roomID == roomID && r->status == "Booked")
            {
                r->status = "Occupied";
                cout << "Room " << roomID << " is now Occupied.\n";
                return;
            }
            r = r->next;
        }
        f = f->right;
    }
    cout << "Room not found or not booked.\n";
}

void cancelLastBooking()
{
    if (historyTop == NULL)
    {
        cout << "\nNo booking available to rollback.\n";
        return;
    }

    int roomID = historyTop->roomID;

    Floor* f = root;
    while (f != NULL)
    {
        Room* r = f->roomHead;
        while (r != NULL)
        {
            if (r->roomID == roomID)
            {
                r->status = "Ready";
                break;
            }
            r = r->next;
        }
        f = f->right;
    }

    cout << "\nRollback (Admin): Booking for Room " << roomID << " cancelled.\n";

    BookingHistory* temp = historyTop;
    historyTop = historyTop->next;
    delete temp;
}

/* ===================== DISPLAY FUNCTIONS ===================== */
void displayHotel(Floor* node)
{
    if (node == NULL)
    {
        return;
    }

    displayHotel(node->left);

    cout << "\n--- FLOOR " << node->floorNo << " ---\n";

    Room* r = node->roomHead;
    while (r != NULL)
    {
        cout << "Room " << r->roomID
             << " | Type: " << r->type
             << " | Status: " << r->status << endl;
        r = r->next;
    }

    displayHotel(node->right);
}

void displayHistory()
{
    if (historyTop == NULL)
    {
        cout << "Booking history is empty.\n";
        return;
    }

    cout << "--- Booking History ---\n";

    BookingHistory* h = historyTop;
    while (h != NULL)
    {
        cout << "Customer: " << h->customer
             << " | Room: " << h->roomID
             << " | Nights: " << h->nights << endl;
        h = h->next;
    }
}

/* ===================== MAIN ===================== */
int main()
{
    int floors = 5;
    int roomsPerFloor = 10;

    // Initialize floors
    for (int i = 1; i <= floors; i++)
    {
        root = insertFloor(root, i);
    }

    // Initialize rooms
    Floor* curr = root;
    for (int f = 1; f <= floors; f++)
    {
        for (int r = 1; r <= roomsPerFloor; r++)
        {
            string type;
            if (r <= 4)
            {
                type = "SINGLE";
            }
            else if (r <= 7)
            {
                type = "DOUBLE";
            }
            else
            {
                type = "SUITE";
            }

            addRoom(curr, f * 100 + r, type);
        }

        curr = curr->right;
    }

    // Make ONLY 1 room unavailable (cannot be booked)
    root->roomHead->status = "Unavailable";

    int choice;

    do
    {
        cout << "\n========== GALAXY HOTEL MANAGEMENT SYSTEM ==========\n";
        cout << "1. Display Hotel Status\n";
        cout << "2. Submit Booking Request\n";
        cout << "3. Process Booking Requests (Batch of 10) [Admin]\n";
        cout << "4. Cancel Last Booking (Rollback) [Admin]\n";
        cout << "5. Display Booking History\n";
        cout << "6. Check-in Customer (Occupy Room)\n";
        cout << "0. Exit\n";

        // Menu input loop
        while (true)
        {
            cout << "Enter your choice: ";
            cin >> choice;

            if (cin.fail() || choice < 0 || choice > 6)
            {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid choice! Enter a number 0-6.\n";
            }
            else
            {
                break;
            }
        }

        if (choice == 1)
        {
            displayHotel(root);
        }
        else if (choice == 2)
        {
            string name;
            string type;
            int nights;
            int p;

            cout << "Customer Name: ";
            cin >> name;

            // Room type loop
            while (true)
            {
                cout << "Room Type (Single/Double/Suite): ";
                cin >> type;
                type = toUpper(type);

                if (validRoomType(type) == true)
                {
                    break;
                }
                else
                {
                    cout << "Invalid room type! Please enter SINGLE, DOUBLE, or SUITE.\n";
                }
            }

            // Nights loop
            while (true)
            {
                cout << "Number of nights: ";
                cin >> nights;

                if (cin.fail() || nights <= 0)
                {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid number of nights! Enter a positive number.\n";
                }
                else if (nights > 30)
                {
                    cout << "Cannot book in advance for more than 30 days.\n";
                }
                else
                {
                    break;
                }
            }

            // Priority loop
            while (true)
            {
                cout << "High Priority Request? (1=Yes, 0=No): ";
                cin >> p;

                if (cin.fail() || (p != 0 && p != 1))
                {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input! Enter 1 for Yes or 0 for No.\n";
                }
                else
                {
                    break;
                }
            }

            enqueueRequest(name, type, nights, p);

            cout << "\nBooking request submitted: "
                 << type.substr(0, 1) + type.substr(1)
                 << " room for " << nights << " nights by " << name << endl;
        }
        else if (choice == 3)
        {
            cout << "\n[Admin] Processing bookings...\n";

            for (int i = 0; i < 10; i++)
            {
                if (highF == NULL && regF == NULL)
                {
                    break;
                }

                processBooking();
            }
        }
        else if (choice == 4)
        {
            cancelLastBooking();
        }
        else if (choice == 5)
        {
            displayHistory();
        }
        else if (choice == 6)
        {
            int roomID;

            while (true)
            {
                cout << "Enter Room ID to check-in: ";
                cin >> roomID;

                if (cin.fail() || roomID <= 0)
                {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid Room ID! Enter a positive number.\n";
                }
                else
                {
                    break;
                }
            }

            checkIn(roomID);
        }

    } while (choice != 0);

    cout << "\nSystem exited successfully.\n";

    return 0;
}


