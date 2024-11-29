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
        oss << left << "|" << setw(10) << bookID << "|"
            << setw(20) << authors << "|"
            << setw(34) << title << "|"
            << setw(11) << releaseDate << "|"
            << setw(10) << fixed << setprecision(2) << price << "|"
            << setw(6) << quantity << "|"
            << setw(12) << saleDate << "|";
        return oss.str();
    }

    static optional<Books> fromFileLine(const string& line) {
        istringstream record(line);
        Books book;

        string ghost, idStr, priceStr, quantityStr;

        if (!(getline(record, ghost, '|') &&
            getline(record, idStr, '|') &&
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

        return book;
    }

private:
   static string trim(const string& str) {
        string trimmed = str;
        trimmed.erase(remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());
        return trimmed;
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
bool isValidNumber(const string& input, bool allowDecimal, bool allowZero);
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

        if (cin >> choice) return choice;
        
        if (!getUserInput(choice)) {
            cout << "Ввод некорректный! Пожалуйста, введите число, согласно пункту меню (0 - 11).";
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
    default: throw InputException(format("{} - некорректный выбор! Пожалуйста, выберите существующий пункт меню.", choice));
    }
}

string getInputWithCancel(const string& prompt) {
    string input;
    cout << prompt << " (или оставьте строку пустой для отмены): ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, input);

    if (input.empty()) { cout << "Действие отменено. Возврат в меню.\n"; }

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

    cout << "\nВыберите способ добавления записи:\n";
    cout << "1. Добавить запись в конец.\n";
    cout << "2. Добавить запись по номеру.\n";
    cout << "0. Отменить добавление записи.\n";
    cout << "Введите ваш выбор: ";

    int choice;
    cin >> choice;
    cin.ignore();

    switch (choice) {
    case 1:
        addRecord(); break;
    case 2:
        addRecordByNumber(); break;
    case 0:
        cout << "Добавление записи отменено.\n"; break;
    default:
        cout << "Неверный выбор. Возврат в главное меню.\n"; break;
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
            file << book.toString() << "\n";
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

    vector<Books> books = readAllBooks(currentFile);
    unsigned int totalRecords = countRecords(books);
    string input;

    cout << format("Введите номер позиции для добавления записи от 1 до {}: ", ++totalRecords);
    cin >> input;
    cin.ignore();
    if (!isValidNumber(input, false, false)) {
        cerr << format("Ошибка: ввод {} недопустим. Номер позиции должен быть натуральным числом.\n", input);
        return;
    }

    unsigned int position = stoi(input) - 1;
    if (position >= totalRecords) {
        cout << format("Позиция {} недопустима для добавления записи. Число должно быть от 1 до {}.\n", ++position, totalRecords);
        return;
    };
    
    cout << "\n";
    try {
        Books book = inputNewRecord();

        if (position < books.size()) books.insert(books.begin() + position, book); 
        else books.push_back(book); 

        writeAllBooks(currentFile, books);
        cout << "Запись успешно добавлена на позицию " << ++position << ".\n";
    }
    catch (const FileException& e) {
        cerr << format("Ошибка при выполнении записи в файл по позиции: {}", ++position) << e.what() << "\n";
    }
}

Books inputNewRecord() {
    Books book;
    string input;

    while (true) {
        cout << "Введите номер книги: ";
        cin >> input;
        cin.ignore();
        if (!isValidNumber(input, false, false)) { 
            cerr << (format("Ошибка: ввод {} недопустим. Номер книги должнен быть натуральным числом.\n", input)); 
            continue;
        }

        try {
            book.bookID = stoi(input);
        }
        catch (const exception& e) {
            throw runtime_error(string("Ошибка: некорректное преобразование типов номера книги для вставки") + e.what());
        }

        if (!isBookIDUnique(book.bookID)) { 
            cerr << (format("Номер книги {} уже существует. Введите уникальный номер.\n", input)); 
            continue;
        }
        break;
    }

    while (true) {
        cout << "Введите авторов (или ? если автор неизвестен): ";
        getline(cin, book.authors);
        if (book.authors.empty()) {
            cerr << ("Строка не может быть пустой. Введите автора.\n");
            continue;
        }

        if (book.authors == "?") book.authors = "Автор неизвестен";
        break;
    }

    while (true) {
        cout << "Введите название книги: ";
        getline(cin, book.title);

        if (!book.title.empty()) break;
        else cerr << ("Строка не может быть пустой. Введите название книги.\n");
    }

    book.releaseDate = inputReleaseDate();

    while (true) {
        cout << "Введите цену: ";
        cin >> input;
        if (!isValidNumber(input, true, false)) {
            cerr << (format("Ошибка: ввод {} недопустим.\nЦена - положительная десятичная дробь с опциональной десятичной частью, содержащая от 1 до 2 цифр.\n", input));
            continue;
        }
        try {
            book.price = stod(input);
        }
        catch (const exception& e) {
            throw runtime_error(string("Ошибка: некорректое преобразование типов цены") + e.what());
        }
        break;
    }

    while (true) {
        cout << "Введите количество: ";
        cin >> input;
        if (!isValidNumber(input, false, false)) {
            cerr << (format("Ошибка: ввод {} недопустим. Количество должно быть натуральным числом.\n", input));
            continue;
        }
        try {
            book.quantity = stoi(input);
        }
        catch (const exception& e) {
            throw runtime_error(string("Ошибка: некорректое преобразование типов количества") + e.what());
        }
        break;
    }

    while (true) {
        cout << "Введите дату продажи в формате ГГГГ-ММ-ДД: ";
        cin >> book.saleDate;
        if (!isValidDate(book.saleDate)) {
            cerr << (format("Ошибка: {} - некорректная дата продажи.", book.saleDate));
            continue;
        }
        break;
    }
    return book;
}

bool isBookIDUnique(int bookID)  {
    try {
        vector<Books> books = readAllBooks(currentFile);
        auto it = find_if(books.begin(), books.end(), [bookID](const Books& book) {
            return book.bookID == bookID;
            });
        return it == books.end();
    }
    catch (const exception&) {
        throw FileException("Ошибка при чтении данных из файла при поиске уникального номера.\n");
        return false;
    }
}

string inputReleaseDate() {
    while (true) {
        cout << "Введите дату выпуска в формате ГГГГ-ММ-ДД, (только год - ГГГГ, или ? - если дата неизвестна): ";
        string inputDate;
        cin >> inputDate;

        if (inputDate == "?") return "Неизвестно";

        if (inputDate.length() == 4) {
            if (!isValidNumber(inputDate, false, false)) { 
                cerr << (format("Ошибка: ввод {} недопустим. Год должнен быть натуральным числом.\n", inputDate)); 
                continue;
            }
            try {
                int year = stoi(inputDate);
                time_t now = time(nullptr);
                tm currentTime;

                if (localtime_s(&currentTime, &now) != 0) {
                    throw runtime_error("Ошибка: не удалось получить текущее время.");
                }
                int currentYear = currentTime.tm_year + 1900;

                if (year < 1000 || year > currentYear) {
                    cerr << ("Ошибка: год должен быть в диапазоне от 1000 до текущего года.\n");
                    continue;
                }
                return inputDate + "-00-00";
            }
            catch (const DateException&) {
                cerr << "Ошибка: некорректный год. Попробуйте снова.\n";
                continue;
            }
        }
        if (isValidDate(inputDate)) return inputDate;
        else cerr << format("Дата \"{}\" введена неверно \n", inputDate);        
    }
}

bool isValidDate(const string& date) {
    using namespace chrono;

    regex dateRegex(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!regex_match(date, dateRegex)) {
        cerr << format("Ошибка: {} не соответствует формату ГГГГ-ММ-ДД. ", date);
        return false;
    }

    istringstream stream(date);
    year_month_day ymd;
    stream >> parse("%F", ymd);

    if (!ymd.ok()) return false;
    
    auto today = year_month_day(floor<days>(system_clock::now()));
    if (ymd > today) {
        cerr << "Введённая дата не должна быть позднее текущей. ";
        return false;
    }
    return true;
}

bool isValidNumber(const string& input, bool allowDecimal, bool allowZero) {
    regex numberRegex;

    if (allowDecimal) numberRegex = allowZero ? regex(R"(^\d+([.,]\d{1,2})?$)") : regex(R"(^([1-9]\d*|0)([.,]\d{1,2})?$)");
    else numberRegex = allowZero ? regex(R"(^\d+$)") : regex(R"(^[1-9]\d*$)");
   
    return regex_match(input, numberRegex);
}

void addTableHeader(ofstream& file) {
    file << left << "|" << setw(10) << "    ID" << "|"
        << setw(20) << "       Авторы" << "|"
        << setw(34) << "             Название" << "|"
        << setw(11) << "Дата выхода" << "|"
        << setw(10) << "   Цена" << "|"
        << setw(6) << "Кол-во" << "|"
        << setw(12) << "Дата продажи" << "|"
        << "\n";
    file << string(111, '-') << "\n";
}

vector<Books> readAllBooks(const string& filename) {
    vector<Books> books;
    ifstream file(filename);
    string line;

    for (int i = 0; i < 2; ++i) {
        if (!getline(file, line)) break;
    }

    while (getline(file, line)) {
        if (auto bookOpt = Books::fromFileLine(line); bookOpt) books.push_back(bookOpt.value());
    }
    return books;
}

void writeAllBooks(const string& filename, const vector<Books>& books) {
    ofstream file(filename);

    file.seekp(0, ios::end);

    if (file.tellp() == 0) addTableHeader(file);

    for (const auto& book : books) {
        file << book.toString() << '\n';
    }
    file.close();
}

unsigned int countRecords(const vector<Books>& books) {
    size_t size = books.size();
    if (size > static_cast<size_t>(numeric_limits<unsigned int>::max())) throw overflow_error("Размер контейнера превышает предел int.");
    
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
    cout << '\n';
    while (getline(file, line)) {
        cout << line << '\n';
    }
    file.close();
}

void deleteRecordByNumber() {
    checkCurrentFile();

    string inputId;
    unsigned int targetNumber;   
    vector<Books> books = readAllBooks(currentFile);
    unsigned int totalRecords = countRecords(books);

    inputId = getInputWithCancel(format("Введите номер записи для удаления от 1 до {}:", totalRecords));
    if (inputId.empty()) return;

    if (!isValidNumber(inputId, false, false)) {
        throw InputException(format("Ошибка: ввод {} недопустим. Номер записи должнен быть натуральным числом.\n", inputId));
        return;
    }

    try {
        targetNumber = stoi(inputId) - 1;
    }
    catch (const exception& e) {
        throw runtime_error(string("Ошибка: некорректное преобразование типов номера записи книги для удаления") + e.what());
    }

    if (targetNumber >= totalRecords) {
        throw InputException(format("Ошибка: номер записи за пределами допустимого диапазона (1 - {}).", totalRecords));
        return;
    }
    books.erase(books.begin() + targetNumber);

    writeAllBooks(currentFile, books);
    cout << format("Запись с номером строки {} успешно удалена.\n", ++targetNumber);
}

void deleteRecordsByReleaseDate() {
    checkCurrentFile();

    string filename = getInputWithCancel("Удаление записей по дате выпуска. Для продолжения нажмите любую клавишу");
    if (filename.empty()) return;

    string targetDate = inputReleaseDate();

    vector<Books> books = readAllBooks(currentFile);
    vector<Books> filteredBooks;
    int deletedCount = 0;

    for (const auto& book : books) {
        if (book.releaseDate != targetDate) filteredBooks.push_back(book);
        else ++deletedCount;
    }

    if (books.size() == filteredBooks.size()) cout << format("Нет записей с датой выпуска {}.\n", targetDate);
    else {
        writeAllBooks(currentFile, filteredBooks);
        cout << format("Записи с датой выпуска \"{}\" удалены. Удалено записей: \"{}\".\n", targetDate, deletedCount);
    }
}

void updateBookPrice() {
    checkCurrentFile();

    unsigned int bookID;
    string newPriceStr;
    string inputId = getInputWithCancel("Введите ID книги для изменения цены");
    if (inputId.empty()) return;

    if (!isValidNumber(inputId, false, false)) throw InputException(format("Ошибка: ввод {} недопустим. Номер книги должнен быть натуральным числом.\n", inputId));

    try {
        bookID = stoi(inputId);
    }
    catch (const exception& e) {
        throw runtime_error(string("Ошибка: некорректное преобразование типов номера книги для изменения цены") + e.what());
    }

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
        cin >> newPriceStr;

        try {
            double newPrice = stod(newPriceStr);
            it->price = newPrice;
        }
        catch (const FileException&) {
            cout << "Ошибка: Некорректное преобразование типов цены для изменения.\n";
            return;
        }

        writeAllBooks(currentFile, books);
        cout << format("Цена книги с номером книги \"{}\" успешно обновлена.\n", bookID);
    }
    catch (const exception& e) {
        cerr << "Непредвиденная ошибка: " << e.what() << "\n";
    }
}

void displayBooksInTimeRange() {
    checkCurrentFile();

    string startDate, endDate;
    startDate = getInputWithCancel("Введите начальную дату в формате ГГГГ-ММ-ДД");
    if (startDate.empty()) return;

    if (!isValidDate(startDate)) {
        throw DateException(format("Начальная дата {} некорректная.\n", startDate));
        return;
    }

    while (true) {
        cout << "Введите конечную дату в формате ГГГГ-ММ-ДД: ";
        cin >> endDate;
        if (!isValidDate(endDate)) {
            cerr << format("Конечная дата {} некорректная.\n", endDate);
            continue;
        }
        break;
    }

    vector<Books> books = readAllBooks(currentFile);
    if (!books.empty()) {
        cout << format("\n|{:<10}|{:<20}|{:<34}|{:<11}|{:<10}|{:<6}|{:<12}|\n",
            "    ID", "       Авторы", "             Название", "Дата выхода", "   Цена", "Кол-во", "Дата продажи");
        cout << string(111, '-') << "\n";
    }

    bool found = false;
    for (const auto& book : books) {
        if (book.saleDate >= startDate && book.saleDate <= endDate) {
            cout << book.toString() << "\n";
            found = true;
        }
    }
    if (!found) cout << format("Записей в диапазоне дат {} - {} не найдено.\n", startDate, endDate);
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
