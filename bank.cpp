#include "bank.h"
#include "utility.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

constexpr double MAX_DEPOSIT = 200000;
constexpr double MAX_WITHDRAW = 50000;
constexpr double MIN_DEPOSIT = 100;
constexpr double MIN_WITHDRAW = 100;
constexpr double MIN_BILL_AMOUNT = 100;
constexpr double MIN_TRANSFER = 100;
constexpr double MAX_TRANSFER = 50000;
constexpr int MAX_PIN_TRIES = 4;
constexpr int MAX_ID_TRIES = 3;
constexpr int PIN_LENGTH = 4;

// Helper Constants
constexpr const char *BAR = "=========================";
constexpr const char *BANK_NAME = "HaBank Buhay";
constexpr const char *INVALID_MSG = "Invalid input.";
constexpr const char *INVALID_MSG_AMOUNT = "That's not a valid number. Please enter a numerical value.";

// Constructors 
ATMUser::ATMUser() {
    this->username = "";
    this->userID = "";
    this->PIN = "";
    this->bal = 0;
}

ATMUser::ATMUser(std::string id, std::string username, std::string pin, double initialBal) {
    this->userID = id;
    this->username = username;
    this->bal = initialBal;
    setPIN(pin); // This will encrypt and set the PIN
}

Log::Log() {
    this->type = "";
    this->amount = 0;
    this->message = "";
    this->refNumber = "";
    this->recipientID = "";
}

Log::Log(std::string type, double amount, std::string message, std::string ref, std::string recipient) {
    this->type = type;
    this->amount = amount;
    this->message = message;
    this->refNumber = ref;
    this->recipientID = recipient;
}

Users::Users() {
    this->refCounter = 1;
}

Menu::Menu() {}

// PIN ENCRYPTION HELPERS

std::string ATMUser::encryptPin(const std::string &plainPin) const {
    std::string encrypted = plainPin;
    for (char &c : encrypted)   
        c += pinShift;
    return encrypted;
}

std::string ATMUser::decryptPin(const std::string &cipheredPin) const {
    std::string decrypted = cipheredPin;
    for (char &c : decrypted)
        c -= pinShift;
    return decrypted;
}

// Getter
std::string ATMUser::getID() const { return userID; }
std::string ATMUser::getUsername() const { return username; }
std::string ATMUser::getPIN() const { return decryptPin(PIN); }
double ATMUser::getBalance() const { return bal; }
std::vector<Log> ATMUser::getLogs() const { return logs; }

std::string Log::getType() const { return type; }
double Log::getAmount() const { return amount; }
std::string Log::getMessage() const { return message; }
std::string Log::getRefNumber() const { return refNumber; }
std::string Log::getRecipientID() const { return recipientID; }
std::string ATMUser::getEncryptedPIN() const { return PIN; }

// Setter
void ATMUser::setID(const std::string &id) { userID = id; }
void ATMUser::setUsername(const std::string &uname) { username = uname; }
void ATMUser::setPIN(const std::string &plainPin) { PIN = encryptPin(plainPin); }
void ATMUser::setEncryptedPIN(const std::string &cipheredPin) { PIN = cipheredPin; }
void ATMUser::setBalance(double balance) { bal = balance; }
void ATMUser::setLogs(const std::vector<Log> &logList) { logs = logList; }

void Log::setType(const std::string &t) { type = t; }
void Log::setAmount(double amt) { amount = amt; }
void Log::setMessage(const std::string &msg) { message = msg; }
void Log::setRefNumber(const std::string &ref) { refNumber = ref; }
void Log::setRecipientID(const std::string &rid) { recipientID = rid; }

void ATMUser::deposit(Users &users) {
    char choice;
    double balance;
    double amount;
    balance = getBalance();

    std::cout << "Your current balance is: Php. " << balance << ".\n";
    while (true)
    {
        std::cout << "Enter amount to deposit: ";
        std::cin >> amount;
        if (!isInputNotValid())
            break;
        std::cout << INVALID_MSG_AMOUNT << std::endl;
    }

    if (amount < MIN_DEPOSIT)
        std::cout << "Deposit amount must be greater than Php. " << MIN_DEPOSIT << '\n';
    else if (amount > MAX_DEPOSIT) 
        std::cout << "Deposit amount must be less than Php. " << MAX_DEPOSIT << '\n';
    else {
        balance += amount;
        setBalance(balance);

        std::cout << "You have deposited Php. " << amount << ".\n";
        std::cout << "Your new balance is Php. "    << std::fixed 
                                                    << std::setprecision(2)
                                                    << balance << ".\n";
        
        addLog(users, "Deposit", amount);
        displayReceipt();
    }
}

void ATMUser::withdraw(Users &users) {
    double amount;
    double balance;
    balance = getBalance();

    std::cout << "Your current balance is: Php. " << balance << ".\n";
    while (true)
    {
        std::cout << "Enter amount to withdraw: ";
        std::cin >> amount;
        if (!isInputNotValid())
            break;
        std::cout << INVALID_MSG_AMOUNT << std::endl;
    }

    if (amount < MIN_WITHDRAW) 
        std::cout << "Withdraw must be greater than Php. " << MIN_WITHDRAW << '\n';
    else if (amount > MAX_WITHDRAW)
        std::cout << "Withdraw must be less than Php. " << MAX_WITHDRAW << '\n';
    else if (amount > balance)
        std::cout << "Insufficient balance to withdraw.\n";
    else {
        balance -= amount;
        setBalance(balance);

        std::cout << "Withdrawal successful. New balance: Php. "    << std::fixed 
                                                                    << std::setprecision(2)
                                                                    << balance << ".\n";
        
        addLog(users, "Withdraw", amount); 
        displayReceipt();    
    }
}

void ATMUser::payBills(Users &users) {
    std::string billName;
    double amount;
    double balance;
    balance = getBalance();

    std::cout << "Your current balance is: Php. " << balance << ".\n";
    std::cout << "Enter bill name: ";
    std::getline(std::cin >> std::ws, billName);
    while (true) {
        std::cout << "Enter amount to pay (Php): ";
        std::cin >> amount;
        if (!isInputNotValid())
            break;
        std::cout << INVALID_MSG_AMOUNT << std::endl;
    }

    if (amount < MIN_BILL_AMOUNT) 
        std::cout << "Invalid bill amount. Must be greater than Php. " << MIN_DEPOSIT << ".\n";
    else if (amount > balance) 
        std::cout << "Transaction failed: Insufficient Balance.\n";
    else {
        balance -= amount;
        setBalance(balance);

        std::cout << "Transaction Success. You have paid Php. " << amount << ". Your new balance is Php. " << std::fixed << std::setprecision(2) << balance << ".\n";
        addLog(users, billName, amount);
        displayReceipt();
    }
}

void ATMUser::displayBalance() {
    std::cout << "Balance Remaining: Php. " << std::fixed << std::setprecision(2) << getBalance() << "\n";
    std::cout << std::endl;
    systemPause();
    return;
}

void ATMUser::transferMoney(Users &users) {
    std::string recipientID, message;
    std::string senderID = getID();
    double balance;
    char confirm;
    ATMUser *user;
    double amount;

    std::cout << "Enter recipient ID (####-####): ";
    std::getline(std::cin >> std::ws, recipientID);
    user = users.findUserByID(recipientID);

    if (!user) {
        std::cout << "Recipient ID does not exist.\n";
        return;
    }

    while (true) {
        std::cout << "Enter amount to transfer: ";
        std::cin >> amount;
        if (isInputNotValid())
            std::cout << INVALID_MSG_AMOUNT << std::endl;
        else if (amount > getBalance())
            std::cout << "Insufficient Balance. Try again.\n";
        else
            break;
    }
    if (amount < MIN_TRANSFER || amount > MAX_TRANSFER) 
        std::cout << "Invalid transfer amount. Must be between " << MIN_TRANSFER<< " and " << MAX_TRANSFER << std::endl;
    else {
        std::cout << "Optional message (Press Enter to skip): ";
        std::cin.ignore();
        std::getline(std::cin, message);
        std::cout << "Sending Php. " << amount << " to " << recipientID << " (" << user->getUsername() << ")\n";
        std::cout << "Message: \"" << message << "\"\n";
        do {
            std::cout << "Confirm? (Y/N): ";
            std::cin >> confirm;
            if (tolower(confirm) != 'y' && tolower(confirm) != 'n') 
                std::cout << "Must be (Y/N) only.\n";
            if (isInputNotValid()) 
                continue;
        } while (tolower(confirm) != 'y' && tolower(confirm) != 'n');
        if (tolower(confirm) != 'y') {
            std::cout << "Transfer cancelled.\n";
            return;
        }

        balance = getBalance();
        balance -= amount;
        setBalance(balance);
        user->setBalance(user->getBalance() + amount);
        std::cout << "Transfer successful. New balance: Php " <<  std::fixed << std::setprecision(2) << balance << ".\n";
        
        user->addLog(users, "Recieved", amount, message, senderID);
        addLog(users, "Transferred", amount, message, recipientID);
        displayReceipt();
    }
}

bool ATMUser::validatePin(const std::string &inputPin) {
    return decryptPin(PIN) == inputPin;
}

void ATMUser::save() {
    std::string folder = "data/" + userID;
    std::string path = folder + "/" + userID + "-data.csv";

    if (!std::filesystem::exists(folder)) {
        std::filesystem::create_directories(folder);
    }

    std::ofstream fout(path);
    if (fout.is_open()) {
        fout << userID << ","
                << username << ","
                << getEncryptedPIN() << ","
                << bal << "\n";
        fout.close();
    } else {
        std::cerr << "[ERROR] Failed to save user data for " << userID;
    }
    return;   
}

void ATMUser::load() {
    std::string path = "data/" + userID + "/" + userID + "-data.csv";

    std::ifstream fin(path);
    if (fin.is_open()) {
        std::string line;
        getline(fin, line);
        std::stringstream ss(line);
        std::string encPin, balStr;

        getline(ss, userID, ',');
        getline(ss, username, ',');
        getline(ss, encPin, ',');
        getline(ss, balStr, ',');

        setEncryptedPIN(encPin);
        bal = std::stod(balStr);

        fin.close();
    } else {
        std::cerr << "[ERROR] Could not load data for " << userID << "\n";
    }
    return;
}

void ATMUser::saveLogs() {
    std::string folder = "data/" + userID;
    std::string path = folder + "/" + userID + "-log.csv";

    if (std::filesystem::exists(folder)) {
        std::filesystem::create_directories(folder);
    }

    std::ofstream fout(path);
    if (fout.is_open()) {
        for (const Log &log : logs) {
            fout << log.getType() << ","
                << log.getAmount() << ","
                << log.getMessage() << ","
                << log.getRefNumber() << ","
                << log.getRecipientID() << "\n";
        }
        fout.close();
    } else {
        std::cerr << "[ERROR] Could not save logs for " << userID << "\n";
    }
}

void ATMUser::loadLogs() {
    logs.clear();

    std::string path = "data/" + userID + "/" + userID + "-log.csv";
    std::ifstream fin(path);
    if (fin.is_open()) {
        std::string line;
        while (getline(fin, line)) {
            std::stringstream ss(line);
            std::string type, amountStr, message, ref, recipient;
            getline(ss, type, ',');
            getline(ss, amountStr, ',');
            getline(ss, message, ',');
            getline(ss, ref, ',');
            getline(ss, recipient, ',');

            double amount = std::stod(amountStr);

            Log log(type, amount, message, ref, recipient);
            logs.push_back(log);
        }
        fin.close();
    } else {
        std::cerr << "[INFO] No log file found for " << userID << ". Skipping load.\n";
    }
}

ATMUser *Users::loginPrompt() {
    std::string userID, enteredPIN;
    bool idFound = false, PINCorrect = false;
    ATMUser *user = nullptr;
    int pinTries = 0, idTries = 0;

    do {
        std::cout << "Enter userID (####-####): ";
        std::cin >> userID;

        user = findUserByID(userID);
        if (!user) {
            std::cout << "userID does not exist.\n";
            idTries++;
            if (idTries >= MAX_ID_TRIES) {
                std::cout << "\nMaximum failed User ID attempts reached.\n";
                std::cout << "Exiting.\n";
                return nullptr;
            }

        }
        else 
            idFound = true;
    } while (!idFound);

    do {
        std::cout << "Enter PIN (####): ";
        std::cin >> enteredPIN;
    
        if (user && user->validatePin(enteredPIN)) {
            std::cout << "Login successful. Welcome, " << user->getUsername() << "!\n";
            PINCorrect = true;
        } else {
            std::cout << "Invalid PIN.\n";
            pinTries++;
            if (pinTries >= MAX_PIN_TRIES) {
                std::cout << "\nMaximum failed PIN attempts reached.\n";
                std::cout << "Exiting.\n";
                return nullptr;
            }
        }
    } while (!PINCorrect);

    return user;
}

std::string Users::getNextUserID() {
    int lastSerial = 0;
    std::ifstream fin("data/uid-counter.txt");

    if (fin.is_open()) {
        fin >> lastSerial;
        fin.close();
    }

    int newSerial = lastSerial + 1;

    std::ofstream fout("data/uid-counter.txt");
    if (fout.is_open()) {
        fout << newSerial;
        fout.close();
    }

    int batchPrefix = 1;
    return formatID(batchPrefix, newSerial);
}

ATMUser *Users::findUserByID(const std::string &id) {
    for (ATMUser &user : userList) {
        if (user.getID() == id) 
            return &user;
    }
    return nullptr;
}

bool Users::addAccount() {
    std::string name, pin;
    double initialBal=0;

    while (true) {
        std::cout << "Enter desired username: ";
        std::getline(std::cin>>std::ws, name);

        if (isNameValid(name)) break;
        else {
            std::cout << "[INVALID INPUT] Only 4 to 64 characters allowed. Try again.\n";
        }
    }

    for (ATMUser &user : userList) {
        if (user.getUsername() == name)
            return false;
    }

    while (true) {
        std::cout << "Enter a 4-character PIN (####): ";
        std::getline(std::cin >> std::ws, pin);

        if (isPinValid(pin)) break;
        else {
            std::cout << "[INVALID INPUT] Only 4 alphanumeric characters allowed. Try again.\n";    
        }
    }

    while (true) {
        std::cout << "Enter initial balance for your new account: Php. ";
        std::cin >> initialBal;
        if (isInputNotValid()) {
            std::cout << "Invalid input. Must be an amount of money. Try again.\n";
        }
        else if (initialBal <= 0) {
            std::cout << "Amount must be a positive number. Try again.\n";
        }
        else {
            break;
        }
    }

    std::string newID = getNextUserID();

    std::cout << "Congratulations! Your account has been successfully created. Your User ID is: " << newID << ". Please keep it safe.\n";

    ATMUser newUser(newID, name, pin, initialBal);
    userList.push_back(newUser);

    newUser.save();
    newUser.saveLogs();

    addToAccountList(newID);

    return true;
}

std::string Users::generateRefNumber() {
    std::ostringstream oss;
    oss << std::setw(6) << std::setfill('0') << refCounter;
    refCounter++;
    return oss.str();
}

void Users::saveAccounts() {
    for (ATMUser &user : userList) {
        user.save();
        user.saveLogs();
    }

    std::ofstream fout("data/ref.txt");
    if (fout.is_open()) {
        fout << refCounter << "\n";
        fout.close(); 
    } else {
        std::cerr << "[ERROR] Could not save reference number.\n";
    }
}

void Users::loadAccounts() {
    userList.clear();

    std::ifstream fin("data/accounts.csv");
    if (!fin.is_open()) {
        std::cerr << "[ERROR] Cannot open accounts list\n";
        return;
    }

    std::string id;
    while (getline(fin, id)) {
        ATMUser user;
        user.setID(id);
        user.load();
        user.loadLogs();
        userList.emplace_back(user);
    }
    fin.close();

    std::ifstream refIn("data/ref.txt");
    if (refIn.is_open()) {
        refIn >> refCounter;
        refIn.close();
    } else {
        refCounter = 1;
    }
}

void Users::addToAccountList(const std::string &id) {
    std::ofstream fout("data/accounts.csv", std::ios::app);
    if (fout.is_open()) {
        fout << id << "\n";
        fout.close();
    }
}

void ATMUser::viewLogs() {
    std::vector<Log> logs = getLogs(); 
    std::string type;

    if (logs.empty())
    {
        std::cout << "No transactions found.\n";
        return;
    }

    for (const Log &log : logs) 
    {
        type = "[" + log.getType() + "]";
        std::cout   << std::setw(16) << std::left << type 
                    << std::setw(6) << std::left << "| Php." 
                    << std::setw(10) << std::left << log.getAmount() << std::endl;

        std::string msg = log.getMessage();
        std::cout   << std::setw(64) << std::left << (msg.empty() ? "N/A" : msg) << std::endl;
        std::cout   << std::setw(6) << std::left << "| Ref: " 
                    << std::setw(10) << std::left << log.getRefNumber();

        std::cout << std::endl << std::endl;
    }

    std::cout << std::endl;
    systemPause();
}

void ATMUser::addLog(Users &users, std::string type, double amount, std::string message, std::string referenceID) {
    std::string ref;
    ref = users.generateRefNumber();

    if (message.empty())
    {
        message = "N/A";
    }

    Log newLog = Log(type, amount, message, ref, referenceID);
    logs.push_back(newLog);
}

// convertBalanceToOtherCurrency(currency)
// currency: a string representing a currency code (e.g., "USD", "JPY", "PHP")
// Output: prints equivalent amount in that currency
void ATMUser::convertBalanceToOtherCurrency(std::string currency) {
    // TODO:

    for (char &c : currency) c = toupper(c);

    double rate = 1.0;
    std::string symbol;

    if (currency == "USD")
    {
        rate = 0.018;
        symbol = "USD. ";
    }
    else if (currency == "JPY")
    {
        rate = 2.6;
        symbol = "Jpy. ";
    }
    else if (currency == "SKW")
    {
        rate = 24.36;
        symbol = "Skw. ";
    }
    else if (currency == "INR")
    {
        rate = 1.51;
        symbol = "Inr";
    }
    else {
        std::cout << "Unsupported currency code.\n";
        return;
    }

    double converted = this->bal * rate;

    std::cout   << "Your balance in " << symbol << ": " << std::fixed << std::setprecision(2)
                << converted << '\n';
    
    return;
}

// ==================== @Sinang ====================
// Handles: Loan System (calculation and interface)

double calculateLoan(double principal, double interestRate, double durationYears) {
    double totalInterest = principal * (interestRate / 100.0) * durationYears;
    return principal + totalInterest;
}

void ATMUser::loanCash(Users &users) {
    bool hasOngoingLoan = false;
    for (const Log &loanLog : logs) {
        if (loanLog.getType() == "Loan" && loanLog.getMessage() == "Loan borrowed") {
            bool paid = false;
            for (const Log &payLog : logs) {
                if (payLog.getType() == "Loan Paid" && payLog.getRefNumber() == loanLog.getRefNumber()) {
                    paid = true;
                    break;
                }
            }
            if (!paid) {
                hasOngoingLoan = true;
                break;
            }
        }
    }
    if (hasOngoingLoan) {
        std::cout << "You have an ongoing loan. Please pay it off before applying for a new one.\n";
        return;
    }

    double principal, durationYears;
    constexpr double interestRate = 1.5; 

    std::cout << "---[ LOAN SERVICE ] ---\n";
    std::cout << "Welcome to the Loan Service!\n";
    std::cout << BAR << "\n";
    while (true) {
        std::cout << "Enter the amount to borrow: ";
        std::cin >> principal;
        if (std::cin.fail() || principal <= 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a positive number.\n";
        } else {
            break;
        }
    }

    while (true) {
        std::cout << "Enter loan duration in years: ";
        std::cin >> durationYears;
        if (!isInputNotValid() && durationYears > 0 && durationYears == (int)durationYears)
            break;
        std::cout << "Invalid input. Please enter a positive integer.\n";
    }

    double totalPayable = calculateLoan(principal, interestRate, durationYears);
    double monthlyPayment = totalPayable / (durationYears * 12);

    std::cout << "Total Payable after " << (int)durationYears << " years: Php "
              << std::fixed << std::setprecision(2) << totalPayable << "\n";
    std::cout << "Monthly Payment: Php " << std::fixed << std::setprecision(2) << monthlyPayment << "\n\n";

    char confirm;
    do {
        std::cout << "Do you want to borrow this loan? (Y/N): ";
        std::cin >> confirm;
        if (isInputNotValid() || (tolower(confirm) != 'y' && tolower(confirm) != 'n')) {
            std::cout << "Must be (Y/N) only.\n";
            continue;
        }
    } while (tolower(confirm) != 'y' && tolower(confirm) != 'n');

    if (tolower(confirm) == 'y') {
        setBalance(getBalance() + principal);
        addLog(users, "Loan", principal, "Loan borrowed", "");
        std::cout << "\nLoan successfully borrowed! Php " << principal << " has been added to your balance.\n";
        std::cout << "Your new balance is Php. " << std::fixed << std::setprecision(2) << getBalance() << ".\n\n";
    } else {
        std::cout << "Loan cancelled.\n";
    }
}

double ATMUser::calculateLoan(double principal, double rate, double durationYears) {
    double totalInterest = principal * (rate / 100.0) * durationYears;
    return principal + totalInterest;
}

void Menu::loanMenu(ATMUser &user, Users &users) {
    user.loanCash(users);
    system("pause");
    return;
}

bool Menu::mainMenuGreetings() {
    char choice;

    std::cout << "Welcome to " << BANK_NAME << "! Your trusted banking partner.\n";
    std::cout << std::endl;
    std::cout << BAR << std::endl;
    std::cout << "=== " << BANK_NAME <<" ===\n"; 
    std::cout << BAR << std::endl;
    do {
        std::cout << "Use ATM (Y/N)? ";
        std::cin >> choice;
        
        if (isInputNotValid()) {
            std::cout << "Invalid Input.\n";
            continue;
        }

        if (tolower(choice) == 'y') 
            return true;
        else if (tolower(choice) == 'n')
            return false;
        
    } while (tolower(choice) != 'y' && tolower(choice) != 'n');

    return false;
}

int Menu::showLoginMenu() {
    int choice;
    do {
        std::cout << "[1] - Login\n";
        std::cout << "[2] - Create Account\n";
        std::cout << "[3] - Exit\n";
        std::cout << "Choose an Option: ";
        std::cin >> choice;

        if (isInputNotValid()) {
            std::cout << "Invalid input\n";
            continue;
        }        
        if (choice > 3 || choice < 1) 
            std::cout << "Invalid option\n";
    } while (choice > 3 || choice < 1);
    
    return choice;
}

void Menu::bankMenu(ATMUser &user, Users &users) {
    int choice;
    users.saveAccounts();
    do {
        std::cout << '\n' << BAR << '\n';
        std::cout << "[1] - My Account\n";
        std::cout << "[2] - Bills & Transfer\n";
        std::cout << "[3] - Loan Services\n";
        std::cout << "[4] - Transaction Log\n";
        std::cout << "[5] - Exit\n";
        std::cout << "Choose an Option: ";
        std::cin >> choice;

        if (isInputNotValid()) {
            std::cout << "Invalid Input.\n";
            continue;
        }
        if (choice > 5 || choice < 1) 
            std::cout << "Invalid Option.\n";
    } while (choice > 5 || choice < 1);

    switch (choice)
    {
        case 1: 
            myAccount(user, users);
            break;
        case 2:
            billsAndTransfer(user, users);
            break;
        case 3:
            loanMenu(user, users);
            break;
        case 4:
            transactionLog(user, users);
            break;
        case 5:
            std::cout << "Thank you for banking with HaBank Buhay. Have a great day!\n";
            return;
    }

    bankMenu(user, users);
}

void Menu::myAccount(ATMUser &user, Users &users) {
    int choice;
    float amount;

    do {
        std::cout << "\n---[ ACCOUNT SERVICES ]---\n";
        std::cout << BAR << "\n";
        std::cout << "[1] - Withdraw\n";
        std::cout << "[2] - Deposit\n";
        std::cout << "[3] - Check Balance\n";
        std::cout << "[4] - Return\n";
        std::cout << "Choose an Option: "; 
        std::cin >> choice;

        if (isInputNotValid()) {
            std::cout << "Invalid input\n";
            continue;
        }

        if (choice > 4 || choice < 1)
            std::cout << "Invalid option.\n";

    } while (choice > 4 || choice < 1);

    switch (choice) 
    {
        case 1:
            user.withdraw(users);
            break;
        case 2:
            user.deposit(users);
            break;
        case 3:
            user.displayBalance();
            break;
        case 4:
            return;
    }
}

void Menu::billsAndTransfer(ATMUser &user, Users &users) {
    int choice;
    do {
        std::cout << "\n---[ BILLS AND TRANSFER ]---\n";
        std::cout << BAR << "\n";
        std::cout << "[1] - Pay Bills\n";
        std::cout << "[2] - Transfer Money\n";
        std::cout << "[3] - Return\n";
        std::cout << "Choose an Option: "; 
        std::cin >> choice;

        if (isInputNotValid()) {
            std::cout << "Invalid input\n";
            continue;
        }

        if (choice > 3 || choice < 1)
            std::cout << "Invalid option.\n";

    } while (choice > 3 || choice < 1);

    switch (choice) 
    {
        case 1:
            user.payBills(users);
            break;
        case 2:
            user.transferMoney(users);
            break;
        case 3:
            return;
    }
}

void Menu::transactionLog(ATMUser &user, Users &users) {
    int choice;

    do {
        std::cout << "\n---[ TRANSACTION LOGS ]---\n";
        std::cout << BAR << "\n";
        std::cout << "[1] - View Logs\n";
        std::cout << "[2] - Return\n";
        std::cout << "Choose an Option: "; 
        std::cin >> choice;

        if (isInputNotValid()) {
            std::cout << "Invalid input\n";
            continue;
        }

        if (choice > 2 || choice < 1)
            std::cout << "Invalid option.\n";

    } while (choice > 2 || choice < 1);

    switch (choice) 
    {
        case 1:
            user.viewLogs();
            break;
        case 2:
            return;
    }
}

void ATMUser::displayReceipt() {
    Log log_item = logs.back();
    char choice;
    do {
        std::cout << "Do you want a copy of your receipt (Y/N)? ";
        std::cin >> choice; 
        if (isInputNotValid()) {
            std::cout << INVALID_MSG << std::endl;
            continue;
        }
        if (tolower(choice) != 'n' && tolower(choice) != 'y')
            std::cout << "Must be (Y/N) only.\n";
    } while (tolower(choice) != 'n' && tolower(choice) != 'y');
    if (tolower(choice) == 'n') return;
    else {
        std::cout << "\n===" << BANK_NAME <<"===\n";
        std::cout << BAR << "\n";
        std::cout << "TRANSACTION RECEIPT\n";
        std::cout << "Type        : " << log_item.getType() << "\n";
        std::cout << "Amount (PHP): " << log_item.getAmount() << "\n";
        std::cout << "User ID     : " << getID() << "\n";
        if (log_item.getType() == "Transferred"){ 
            std::cout << "Recipient ID: " << log_item.getRecipientID() << "\n"; 
        }  
        std::cout << "Username    : " << getUsername() << "\n";
        std::cout << "Reference # : " << log_item.getRefNumber() << "\n";
        std::cout << "Your New Balance is Php. " << std::fixed << std::setprecision(2) << getBalance() << "\n";
        std::cout << "Thank you!\n";
        
        std::cout << std::endl;
        systemPause();
    }
}
