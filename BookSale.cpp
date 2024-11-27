// BookSale.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <stdexcept>
#include <limits>
#include <filesystem>
#include <vector>
#include <regex> 
#include <iomanip>
#include <sstream>
#include <windows.h>

using namespace std;
using namespace std::filesystem;

class Books {
public:
    unsigned int bookID = 0;
    string authors = "Author";
    string title = "Title";
    string releaseDate = "0000-00-00";
    double price = 0.0;
    int quantity = 0;
    string saleDate = "0000-00-00";

    string toString() const {
        ostringstream oss;
        oss << left << setw(10) << bookID << "|"
            << setw(20) << authors << "|"
            << setw(30) << title << "|"
            << setw(15) << releaseDate << "|"
            << setw(10) << fixed << setprecision(2) << price << "|"
            << setw(10) << quantity << "|"
            << setw(15) << saleDate << "|";
        return oss.str();
    }

    static optional<Books> fromFileLine(const string& line) {
        istringstream record(line);
        Books book;

        string idStr, priceStr, quantityStr;
        if (!(getline(record, idStr, '|') &&
            getline(record, book.authors, '|') &&
            getline(record, book.title, '|') &&
            getline(record, book.releaseDate, '|') &&
            getline(record, priceStr, '|') &&
            getline(record, quantityStr, '|') &&
            getline(record, book.saleDate, '|'))) {
            throw invalid_argument("Некорректная строка файла.");
        }

        book.bookID = stoi(trim(idStr));
        book.price = stod(trim(priceStr));
        book.quantity = stoi(trim(quantityStr));
        book.releaseDate = trim(book.releaseDate);
        book.saleDate = trim(book.saleDate);
        book.authors = trim(book.authors);
        book.title = trim(book.title);

        return book;
    }

private:
    static string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == string::npos || last == string::npos) ? "" : str.substr(first, last - first + 1);
    }
};

enum MenuOption {
    CREATE_FILE = 1,
    SELECT_FILE,
    ADD_RECORD,
    DISPLAY_RECORDS,
    DELETE_RECORD_BY_NUMBER,
    DELETE_RECORDS_BY_DATE,
    UPDATE_BOOK_PRICE,
    DISPLAY_BOOKS_IN_TIME_RANGE,
    BACKUP_FILE,
    RESTORE_FILE,
    DELETE_FILE,
    EXIT = 0
};

class FileException : public runtime_error {
public:
    FileException(const string& message) : runtime_error(message) {}
};

class InputException : public invalid_argument {
public:
    InputException(const string& message) : invalid_argument(message) {}
};

class DateException : public logic_error {
public:
    DateException(const string& message) : logic_error(message) {}
};


string currentFile;

void createFile();
void selectFile();
void checkCurrentFile();
void addRecord();
void addRecordByNumber();
void displayRecords();
void deleteRecordByNumber();
void deleteRecordsByReleaseDate();
void updateBookPrice();
void displayBooksInTimeRange();
void backupFile();
void restoreFileFromBackup();
void deleteFile();

void setConsoleParams();
void runApplication();
int displayMenu();
bool getUserInput(int& input);
void processUserChoice(int choice);
string getInputWithCancel(const string& prompt);
bool confirmAction(const string& prompt);
void addRecordMenu();
Books inputNewRecord();
bool isBookIDUnique(int bookID);
string inputReleaseDate();
bool isValidDate(const string& date);
void addTableHeader(ofstream& file);
vector<Books> readAllBooks(const string& filename);
void writeAllBooks(const string& filename, const vector<Books>& lines);
unsigned int countRecords(const vector<Books>& books);


int main() {
    setConsoleParams();

    try {
        runApplication();
    }
    catch (const exception& e) {
        cerr << "Произошла ошибка приложения: " << e.what() << endl;
    }
    return 0;
}

void setConsoleParams() {
    setlocale(LC_ALL, "RU");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
}

void runApplication() {
    int choice;
    do {
        choice = displayMenu();
        try {
            processUserChoice(choice);
        }
        catch (const exception& e) {
            cerr << "Ошибка при выполнении действия: " << e.what() << endl;
        }
    } while (choice != 0);
}

int displayMenu() {
    int choice;
    while (true) {
        cout << "\nМеню:\n";
        cout << "1. Создать новый файл\n";
        cout << "2. Выбрать файл для работы\n";
        cout << "3. Добавить запись в файл\n";
        cout << "4. Вывести все записи\n";
        cout << "5. Удалить запись по номеру\n";
        cout << "6. Удалить записи по дате выпуска\n";
        cout << "7. Изменить цену книги\n";
        cout << "8. Вывести книги в промежутке времени\n";
        cout << "9. Скопировать данные в резервный файл\n";
        cout << "10. Восстановить файл из резервного\n";
        cout << "11. Удалить файл\n";
        cout << "0. Выход\n";
        cout << "Ваш выбор: ";

        if (cin >> choice) {
            return choice;
        }
        if (!getUserInput(choice)) {
            cout << ("Некорректный ввод! Пожалуйста, введите число.");
        }
        
    }
}

bool getUserInput(int& input) {
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}

void processUserChoice(int choice) {
    MenuOption option = static_cast<MenuOption>(choice);

    switch (option) {
    case CREATE_FILE: createFile(); break;
    case SELECT_FILE: selectFile(); break;
    case ADD_RECORD: addRecordMenu(); break;
    case DISPLAY_RECORDS: displayRecords(); break;
    case DELETE_RECORD_BY_NUMBER: deleteRecordByNumber(); break;
    case DELETE_RECORDS_BY_DATE: deleteRecordsByReleaseDate(); break;
    case UPDATE_BOOK_PRICE: updateBookPrice(); break;
    case DISPLAY_BOOKS_IN_TIME_RANGE: displayBooksInTimeRange(); break;
    case BACKUP_FILE: backupFile(); break;
    case RESTORE_FILE: restoreFileFromBackup(); break;
    case DELETE_FILE: deleteFile(); break;
    case EXIT: cout << "Выход из программы.\n"; break;
    default: throw InputException("Некорректный выбор! Пожалуйста, выберите существующий пункт меню.");
    }
}

string getInputWithCancel(const string& prompt) {
    string input;
    cout << prompt << " (или оставьте строку пустой для отмены): ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, input);

    if (input.empty()) {
        cout << "Действие отменено. Возврат в меню.\n";
    }
    return input;
}

void createFile() {
    string filename = getInputWithCancel("Введите имя файла для работы");
    if (filename.empty()) return;

    if (exists(filename)) {
        if (!confirmAction(format("Файл \"{}\" уже существует. Все записи этого файла будут уничтожены! Перезаписать?", filename))) {
            cout << "Файл не был создан.\n";
            return;
        }
    }

    ofstream newFile(filename, ios::trunc);
    if (!newFile) {
        throw FileException(format("Не удалось создать файл \"{}\".", filename));
    }
    cout << format("Файл \"{}\" успешно создан.\n", filename);

    currentFile = filename;
}

void selectFile() {
    string filename = getInputWithCancel("Введите имя файла для работы");
    if (filename.empty()) return;

    if (!exists(filename)) {
        if (!confirmAction(format("Файл \"{}\" не существует. Создать его?", filename))) {
            cout << "Выбор файла отменён.\n";
            return;
        }
        ofstream newFile(filename);
        if (!newFile) {
            throw FileException(format("Не удалось создать файл \"{}\".", filename));
        }
        cout << format("Файл \"{}\" успешно создан.\n", filename);
    }

    currentFile = filename;
    cout << format("Файл \"{}\" выбран для работы.\n", filename);
}

bool confirmAction(const string& prompt) {
    char response;
    cout << prompt << " (y/любая другая клавиша): ";
    cin >> response;
    return response == 'y' || response == 'Y';
}

void checkCurrentFile() {
    if (currentFile.empty()) {
        throw FileException("Файл не выбран. Пожалуйста, выберите файл из меню 2. \"Выбрать файл для работы\"");
    }
    if(!filesystem::exists(currentFile)) {
        ofstream file(currentFile);
        if (!file) {
            throw FileException(format("Ошибка создания файла \"{}\".", currentFile));
        }
        file.close();
    }
}

void addRecordMenu() {
    checkCurrentFile();

    cout << "Выберите способ добавления записи:\n";
    cout << "1. Добавить запись в конец.\n";
    cout << "2. Добавить запись по номеру.\n";
    cout << "0. Отменить добавление записи.\n";
    cout << "Введите ваш выбор: ";

    int choice;
    cin >> choice;
    cin.ignore();

    switch (choice) {
    case 1:
        addRecord();
        break;
    case 2:
        addRecordByNumber();
        break;
    case 0:
        cout << "Добавление записи отменено.\n";
        break;
    default:
        cout << "Неверный выбор. Возврат в главное меню.\n";
        break;
    }
}

void addRecord() {
    checkCurrentFile();

    ofstream file(currentFile, ios::app);

    file.seekp(0, ios::end);
    if (file.tellp() == 0) {
        addTableHeader(file);
    }

    cout << "Начало ввода записей. Введите данные книги или * для завершения ввода.\n";
    while (true) {
        cout << "\nВведите * для завершения или любую клавишу для добавления записи: ";
        string choice;
        cin >> choice;
        if (choice == "*") {
            cout << "Ввод записей завершён.\n";
            break;
        }

        try {
            Books book = inputNewRecord();
            file << book.toString();
            cout << "Запись успешно добавлена.\n";
        }
        catch (const FileException& e) {
            cerr << "Ошибка при выполнении записи в конец файла: " << e.what() << "\n";
            continue;
        }
    }
    file.close();
}

void addRecordByNumber() {
    checkCurrentFile();
    string filename = getInputWithCancel("Введите номер позиции для добавления записи ");
    if (filename.empty()) return;

    int position;
    cin >> position;
    cin.ignore();

    vector<Books> books = readAllBooks(currentFile);
    int totalRecords = countRecords(books);

    if (position < 0 || position > totalRecords) {
        cout << format("Позиция {} недопустима для добавления записи. Разница с количеством записей {} должна быть не более 1.\n", position, totalRecords);
        return;
    };
    
    try {
        Books book = inputNewRecord();
        if (position < books.size()) {
            books.insert(books.begin() + position, book);
        }
        else {
            books.push_back(book);
        }

        writeAllBooks(currentFile, books);
        cout << "Запись успешно добавлена на позицию " << position << ".\n";
    }
    catch (const FileException& e) {
        cerr << "Ошибка при выполнении записи в файл по позиции: " << e.what() << "\n";
    }
}

Books inputNewRecord() {
    Books book;
    cout << "\nВведите номер книги: ";
    string input;
    cin >> input;
    cin.ignore();

    try {
        book.bookID = stoi(input);

        if (!isBookIDUnique(book.bookID)) {
            throw InputException("Номер книги уже существует. Введите уникальный номер.");
        }
    }
    catch (const exception& e) {
        throw runtime_error(string("Ошибка ввода номера книги: ") + e.what());
    }

    cout << "Введите авторов (или ? если автор неизвестен): ";
    getline(cin, book.authors);
    if (book.authors == "?") {
        book.authors = "Неизвестно";
    }

    cout << "Введите название книги: ";
    getline(cin, book.title);

    book.releaseDate = inputReleaseDate();

    cout << "Введите цену: ";
    cin >> input;
    try {
        book.price = stod(input);
    }
    catch (const exception&) {
        throw runtime_error("Ошибка ввода цены. Введите число.");
    }

    cout << "Введите количество: ";
    cin >> input;
    try {
        book.quantity = stoi(input);
    }
    catch (const exception&) {
        throw runtime_error("Ошибка ввода количества. Введите целое число.");
    }

    cout << "Введите дату продажи (ГГГГ-ММ-ДД): ";
    cin >> book.saleDate;
    if (!isValidDate(book.saleDate)) {
        throw runtime_error("Некорректная дата продажи.");
    }
    return book;
}

bool isBookIDUnique(int bookID) {
    ifstream file(currentFile);
    if (!file) {
        throw FileException(format("Ошибка открытия файла \"{}\" для проверки уникальности.", currentFile));
    }

    string line;

    for (int i = 0; i < 2; ++i) {
        if (!getline(file, line)) {
            break;
        }
    }

    while (getline(file, line)) {
        istringstream record(line);
        string idStr;
        if (getline(record, idStr, '|')) {
            try {
                int existingID = stoi(idStr);
                if (existingID == bookID) {
                    file.close();
                    return false;
                }
            }
            catch (const exception&) {
                cerr << "Ошибка при чтении данных из файла.\n";
                file.close();
                return false;
            }
        }
    }
    file.close();
    return true;
}

string inputReleaseDate() {
    while (true) {
        cout << "Введите дату выпуска (ГГГГ-ММ-ДД, только год ГГГГ, или ? если дата неизвестна): ";
        string date;
        cin >> date;

        if (date == "?") {
            return "Неизвестно";
        }

        if (date.length() == 4) {
            try {
                int year = stoi(date);
                if (year < 1000 || year > 9999) {
                    throw InputException("Год должен быть 4-значным числом.");
                }
                return date + "-00-00";
            }
            catch (const DateException&) {
                cerr << "Ошибка: некорректный год. Попробуйте снова.\n";
                continue;
            }
        }

        if (isValidDate(date)) {
            return date;
        }
        else {
            cerr << format("Дата \"{}\" введена неверно \n", date);
        }
    }
}

bool isValidDate(const string& date) {
    using namespace chrono;

    regex dateRegex(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!regex_match(date, dateRegex)) {
        cout << "Не соответствует формату ГГГГ-ММ-ДД. ";
        return false; // Формат не соответствует
    }

    istringstream stream(date);
    year_month_day ymd;
    stream >> parse("%F", ymd);

    if (!ymd.ok()) {
        return false;
    }

    auto today = year_month_day(floor<days>(system_clock::now()));

    if (ymd > today) {
        cout << "Введённая дата не должна быть позднее текущей. ";
        return false;
    }
    return true;
}

void addTableHeader(ofstream& file) {
    file << left << setw(10) << "ID" << "|"
        << setw(20) << "Авторы" << "|"
        << setw(30) << "Название" << "|"
        << setw(15) << "Дата выхода" << "|"
        << setw(10) << "Цена" << "|"
        << setw(10) << "Кол-во" << "|"
        << setw(15) << "Дата продажи" << "|"
        << "\n";
    file << string(110, '-') << "\n";
}

vector<Books> readAllBooks(const string& filename) {
    vector<Books> books;
    ifstream file(filename);
    string line;
    int lineCount = 0;

    while (getline(file, line)) {
        ++lineCount;
        if (lineCount <= 2) { continue; }

        if (auto bookOpt = Books::fromFileLine(line); bookOpt) {
            books.push_back(bookOpt.value());
        }
    }
    return books;
}

void writeAllBooks(const string& filename, const vector<Books>& books) {
    ofstream file(filename);

    file.seekp(0, ios::end);
    if (file.tellp() == 0) {
        addTableHeader(file);
    }

    for (const auto& book : books) {
        file << book.toString() << '\n';
    }
    file.close();
}

unsigned int countRecords(const vector<Books>& books) {
    size_t size = books.size();
    if (size > static_cast<size_t>(numeric_limits<unsigned int>::max())) {
        throw overflow_error("Размер контейнера превышает предел int.");
    }
    return static_cast<unsigned int>(size);
}

void displayRecords() {
    checkCurrentFile();

    ifstream file(currentFile);
    if (!file) {
        throw FileException(format("Ошибка: не удалось открыть файл \"{}\" для чтения.\n", currentFile));
        return;
    }

    string line;
    while (getline(file, line)) {
        cout << line << '\n';
    }
    file.close();
}

void deleteRecordByNumber() {
    checkCurrentFile();
    string filename = getInputWithCancel("Введите номер записи для удаления ");
    if (filename.empty()) return;
   
    int targetNumber;
    cin >> targetNumber;

    targetNumber = --targetNumber;
    vector<Books> books = readAllBooks(currentFile);
    
    int totalRecords = countRecords(books);

    if (targetNumber < 0 || targetNumber >= totalRecords) { throw InputException(format("Ошибка: номер записи за пределами допустимого диапазона (1 - {}).\n", totalRecords)); }

    books.erase(books.begin() + targetNumber);

    writeAllBooks(currentFile, books);
    cout << format("Запись с номером \"{}\" успешно удалена.\n", targetNumber + 1);
}

void deleteRecordsByReleaseDate() {
    checkCurrentFile();
    string filename = getInputWithCancel("Удаление записей по дате выпуска \n");
    if (filename.empty()) return;

    string targetDate = inputReleaseDate();

    vector<Books> books = readAllBooks(currentFile);
    vector<Books> filteredBooks;
    int deletedCount = 0;

    for (const auto& book : books) {
        if (book.releaseDate != targetDate) {
            filteredBooks.push_back(book);
        }
        else {
            ++deletedCount;
        }
    }

    if (books.size() == filteredBooks.size()) {
        cout << format("Нет записей с датой выпуска {}.\n", targetDate);
    }
    else {
        writeAllBooks(currentFile, filteredBooks);
        cout << format("Записи с датой выпуска \"{}\" удалены. Удалено записей: \"{}\".\n", targetDate, deletedCount);
    }
}

void updateBookPrice() {
    checkCurrentFile();
    string filename = getInputWithCancel("Введите ID книги для изменения цены ");
    if (filename.empty()) return;

    int bookID;
    cin >> bookID;
    cin.ignore();

    try {
        vector<Books> books = readAllBooks(currentFile);
        auto it = find_if(books.begin(), books.end(), [bookID](const Books& book) {
            return book.bookID == bookID;
            });
        
        if (it == books.end()) {
            cout << format("Книга с ID \"{}\" не найдена.\n", bookID);
            return;
        }

        cout << "Введите новую цену книги: ";
        string newPriceStr;
        cin >> newPriceStr;

        try {
            double newPrice = stod(newPriceStr);
            if (newPrice < 0) {
                cout << "Ошибка: Цена не может быть отрицательной.\n";
                return;
            }
            it->price = newPrice;
        }
        catch (const FileException&) {
            cout << "Ошибка: Некорректное значение для цены.\n";
            return;
        }

        writeAllBooks(currentFile, books);
        cout << format("Цена книги с ID \"{}\" успешно обновлена.\n", bookID);
    }
    catch (const runtime_error& e) {
        cerr << "Ошибка при выполнении действия: " << e.what() << "\n";
    }
    catch (const exception& e) {
        cerr << "Непредвиденная ошибка: " << e.what() << "\n";
    }
}

void displayBooksInTimeRange() {
    checkCurrentFile();

    string startDate, endDate;

    cout << "Введите начальную дату в формате ГГГГ-ММ-ДД: ";
    cin >> startDate;
    if (!isValidDate(startDate)) {
        cerr << format("Ошибка: начальная дата {} некорректная.\n", startDate);
        return;
    }

    cout << "Введите конечную дату в формате ГГГГ-ММ-ДД: ";
    cin >> endDate;
    if (!isValidDate(endDate)) {
        cerr << format("Ошибка: конечная дата {} некорректная.\n", endDate);
        return;
    }

    ifstream file(currentFile);
    string line;
    int lineCount = 0;
    bool found = false;

    while (getline(file, line)) {
        ++lineCount;

        if (lineCount == 1 || lineCount == 2) {
            cout << line << "\n";
            continue;
        }

        auto bookOpt = Books::fromFileLine(line);
        if (bookOpt) {
            Books book = bookOpt.value();
            if (book.saleDate >= startDate && book.saleDate <= endDate) {
                cout << book.toString() << '\n';
                found = true;
            }
        }
    }

    if (!found) {
        cout << format("Нет записей о проданных книгах в {} - {} промежуток времени.\n", startDate, endDate);
    }

    file.close();
}

void backupFile() {
    checkCurrentFile();

    string backupFile = currentFile + "_backup";

    try {
        copy_file(currentFile, backupFile, copy_options::overwrite_existing);
    }
    catch (const filesystem_error& e) {
        throw FileException(format("Ошибка при создании резервной копии: {}", e.what()));
    }
    cout << format("Данные успешно скопированы в резервный файл: \"{}\".\n", backupFile);
}

void restoreFileFromBackup() {
    checkCurrentFile();

    string backupFile = currentFile + "_backup";

    try {
        if (!exists(backupFile)) {
            throw FileException(format("Резервный файл \"{}\" не найден.", backupFile));
        }

        if (file_size(backupFile) == 0) {
            if (!confirmAction(format("Резервный файл \"{}\" пуст. Вы уверены, что хотите продолжить восстановление?", backupFile))) {
                cout << "Выбор файла отменён.\n";
                return;
            }
        }

        copy_file(backupFile, currentFile, copy_options::overwrite_existing);
        cout << format("Файл \"{}\" успешно восстановлен из резервного файла.\n", currentFile);
    }
    catch (const filesystem_error& e) {
        cerr << "Ошибка при работе с файлами: " << e.what() << "\n";
    }
    catch (const FileException& e) {
        cerr << "Ошибка: " << e.what() << "\n";
    }
    catch (const exception& e) {
        cerr << "Непредвиденная ошибка: " << e.what() << "\n";
    }
}

void deleteFile() {
    checkCurrentFile();

    cout << format("Вы действительно хотите удалить файл \"{}\"? (y/любая клавиша): ", currentFile);
    char confirmation;
    cin >> confirmation;

    if (tolower(confirmation) != 'y') {
        cout << "Удаление файла отменено.\n";
        return;
    }

    if (remove(currentFile.c_str()) != 0) {
        perror((format("Ошибка при удалении файла \"{}\"", currentFile)).c_str());
        return;
    }

    cout << format("Файл \"{}\" успешно удалён.\n", currentFile);

    currentFile.clear();
}


// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
