#ifndef BOOK_H
#define BOOK_H
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <chrono>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <memory>



// MySQL Headers
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>


using namespace std;
namespace fs = std::filesystem;

struct Book {
	int bookId = 0, numberOfCopies = 0, availableCopies = 0;
	string title, author;
	bool isAvailable = false;
};

struct Student {
	string studentName, fatherName, section, email, issueDate, dueDate;
	int rollNumber = 0, days = 0;
};

class LibraryBook {
public:
	LibraryBook() :con(nullptr) {
		if (!fs::exists("book")) {
			fs::create_directories("book");
		}
	};
	void addBook(sql::Connection* con) {
		Book b;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		cout << "\n--- Add New Book To Database ---" << endl;
		cout << "Enter Book Title : ";
		getline(cin, b.title);

		cout << "Enter Author Name : ";
		getline(cin, b.author);

		cout << "Enter the number of copies : ";
		cin >> b.numberOfCopies;
		b.availableCopies = b.numberOfCopies;

		b.isAvailable = true;

		try {
			unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
				"INSERT INTO books(title, author, is_available, total_copies, available_copies) VALUES (?, ?, ?, ?, ?)"
			));

			pstmt->setString(1, b.title);
			pstmt->setString(2, b.author);
			pstmt->setBoolean(3, b.isAvailable);
			pstmt->setInt(4, b.numberOfCopies);
			pstmt->setInt(5, b.availableCopies);

			pstmt->executeUpdate();
			cout << "Book added successfully to SQL database!" << endl;
		}
		catch (sql::SQLException& e) {
			cout << "Database Error while adding book: " << e.what() << endl;
		}
	}

	void viewBooks(sql::Connection* con)
	{
		try {
			unique_ptr<sql::Statement> stmt(con->createStatement());

			unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM books"));

			cout << "\n-----------------------------------------------------" << endl;
			while (res->next()) {
				cout << "ID    : " << res->getInt("book_id") << endl;
				cout << "TITLE : " << res->getString("title") << endl;
				cout << "Author: " << res->getString("author") << endl;
				if (res->getInt("available_copies") > 0) cout << "STATUS: [Available]\n";
				else cout << "STATUS: [Not Available]\n";
				cout << res->getInt("available_copies") << " Copies Available Out of " << res->getInt("total_copies");
				cout << "\n-----------------------------------------------------" << endl;
			}

		}
		catch (sql::SQLException& e) {
			cout << "Error viewing books: " << e.what() << endl;
		}
	}

	void findBook(sql::Connection* con)
	{
		string searchTitle;
		cout << "Enter Book Title to search: ";
		getline(cin, searchTitle);

		try {
			unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
				"SELECT * FROM books WHERE title LIKE ?"
			));

			pstmt->setString(1, "%" + searchTitle + "%");
			unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
			bool found = false;

			while (res->next()) {
				found = true;
				cout << "\n-------------------------------------------\n";
				cout << "Book Found in Database!\n";
				cout << "Book ID	: " << res->getInt("book_id") << endl;
				cout << "Title		: " << res->getString("title") << endl;
				cout << "Author		: " << res->getString("author") << endl;
				cout << "Status		: " << (res->getBoolean("is_available") ? "Available" : "Issued") << endl;
				cout << "\n-------------------------------------------\n";
			}
			if (!found) {
				cout << "No books found containing: " << searchTitle << endl;
			}
		}
		catch (sql::SQLException& e) {
			cout << "Database Error: " << e.what() << endl;
		}
	}

	void issueBook(sql::Connection* con) {
		string searchTitle;
		cout << "Enter Book Title to Issue: ";
		getline(cin, searchTitle);

		try {
			unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
				"SELECT * FROM books WHERE title = ? AND is_available = 1"
			));

			pstmt->setString(1, searchTitle);
			unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

			if (res->next()) {
				int bId = res->getInt("book_id");
				string bTitle = res->getString("title");
				string bAtuhor = res->getString("author");

				cout << "\t\t\tBook is available! Procedding with issue...\n";

				Student s;
				cout << "Enter Student Name : ";
				getline(cin, s.studentName);

				cout << "Enter Roll Number : ";
				cin >> s.rollNumber;

				cout << "Enter Student Class Section : ";
				cin >> s.section;
				cin.ignore(numeric_limits<streamsize>::max(), '\n');

				cout << "Enter Father's Name : ";
				getline(cin, s.fatherName);

				cout << "Enter your gmail : ";
				getline(cin, s.email);

				if (s.email.find("@gmail.com") == string::npos) {
					cout << "Invalid Gmail address.\n";
					return;
				}

				cout << "Enter number of days to issue: ";
				cin >> s.days;

				s.issueDate = getCurrentDate();
				s.dueDate = calculateDueDate(s.days);

				unique_ptr<sql::PreparedStatement> updateStmt(con->prepareStatement(
					"UPDATE books SET available_copies = available_copies - 1 WHERE book_id = ? AND available_copies > 0"
				));
				updateStmt->setInt(1, bId);
				updateStmt->executeQuery();

				unique_ptr<sql::PreparedStatement> checkStmt(con->prepareStatement(
					"UPDATE books SET is_available = 0 WHERE book_id = ? AND available_copies = 0"
				));
				checkStmt->setInt(1, bId);
				checkStmt->executeUpdate();

				unique_ptr<sql::PreparedStatement> recordStmt(con->prepareStatement(
					"INSERT INTO issued_books(book_id, book_title, student_name, roll_number, section, issue_date, due_date, email, father_name, days_issued)"
					"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?,?)"
				));

				recordStmt->setInt(1, bId);
				recordStmt->setString(2, bTitle);
				recordStmt->setString(3, s.studentName);
				recordStmt->setInt(4, s.rollNumber);
				recordStmt->setString(5, s.section);
				recordStmt->setString(6, s.issueDate);
				recordStmt->setString(7, s.dueDate);
				recordStmt->setString(8, s.email);
				recordStmt->setString(9, s.fatherName);
				recordStmt->setInt(10, s.days);

				recordStmt->executeUpdate();

				cout << "Issued Book Data save successfully!\n";

				Book b;
				b.bookId = bId;
				b.title = bTitle;
				b.author = bAtuhor;

				string name = s.studentName;
				for (char& c : name) {
					if (c == ' ')
						c = '_';
				}

				name += "_" + to_string(s.rollNumber) + "_receipt";
				generateReceiptHTML(s, b, name);

				string command = "start book/" + name + ".html";
				system(command.c_str());

				convertHTMLtoPDF(name);

				try {
					unique_ptr<sql::PreparedStatement> emailStmt(
						con->prepareStatement(
							"INSERT INTO email_queue(student_email, receipt_name) VALUE(?, ?)"
						)
					);
					emailStmt->setString(1, s.email);
					emailStmt->setString(2, name);
					emailStmt->executeUpdate();
					cout << "Email added to queue.\n";
				}
				catch (sql::SQLException& e) {
					cout << "Error inserting into email_queue: " << e.what() << endl;
				}
			}
			else {
				cout << "Soory, this book is either not in the library or already issued.\n";
			}
		}
		catch (sql::SQLException& e) {
			cout << "SQL Error: " << e.what() << endl;
		}
	}

	void returnBook(sql::Connection* con) {
		this->con = con;
		int bookId, rollNum;
		string section;
		cout << "Enter Book ID to Return: "; cin >> bookId;
		cout << "Enter Student Roll Number: "; cin >> rollNum;
		cout << "Enter Section (e.g., A, B, C, D): "; cin >> section;

		if (!isBookIssued(bookId,rollNum,section)) {
			cout << "There is no book issued on this ID.\n";
			return;
		}
		Student s;
		Book b;

		if (!getIssueDetailsByBookId(bookId,rollNum,section, s, b)) {
			cout << "Failed to retrieve issue details\n";
			return;
		}

		string currentDate = getCurrentDate();
		int currentTotalDays = dateToDays(currentDate);
		int dueTotalDays = dateToDays(s.dueDate);

		int fine = 0;
		if (currentTotalDays > dueTotalDays) {
			int delay = currentTotalDays - dueTotalDays;
			fine = delay * 5;
			cout << "LATE RETURN: " << delay << " days overdue. Fine: " << fine << endl;
		}
		else {
			cout << "Returned on time. No fine.\n";
		}


		try {
			unique_ptr<sql::PreparedStatement> logStmt(con->prepareStatement(
				"INSERT INTO return_log(roll_number, section, father_name, email, student_name, book_id, book_title, issue_date, due_date,return_date,fine_amount) "
				"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?,?)"
			));
			logStmt->setInt(1, s.rollNumber);
			logStmt->setString(2, s.section);
			logStmt->setString(3, s.fatherName);
			logStmt->setString(4, s.email);
			logStmt->setString(5, s.studentName);
			logStmt->setInt(6, b.bookId);
			logStmt->setString(7, b.title);
			logStmt->setString(8, s.issueDate);
			logStmt->setString(9, s.dueDate);
			logStmt->setString(10, currentDate);
			logStmt->setInt(11, fine);
			logStmt->executeUpdate();

			unique_ptr<sql::PreparedStatement> delStmt(con->prepareStatement(
				"DELETE FROM issued_books WHERE book_id = ? AND roll_number = ? AND section = ?"
			));
			delStmt->setInt(1, bookId);
			delStmt->setInt(2, rollNum);
			delStmt->setString(3, section);
			delStmt->executeUpdate();


			unique_ptr<sql::PreparedStatement> upStmt(con->prepareStatement(
				"UPDATE books SET "
				"available_copies = available_copies + 1, "
				"is_available = 1 "
				"WHERE book_id = ? AND available_copies < total_copies"
			));

			upStmt->setInt(1, bookId);
			upStmt->executeUpdate();
			cout << "Database updated successfully\n";
		}
		catch (sql::SQLException& e) {
			cout << "Critical SQL Error: " << e.what() << endl;
			return;
		}

		try {
			unique_ptr<sql::PreparedStatement> emailStmt(
				con->prepareStatement(
					"INSERT INTO return_email_queue(student_email, student_name, book_title, book_id, return_date, fine_amount) "
					"VALUES (?, ?, ?, ?, ?, ?)"
				)
			);

			emailStmt->setString(1, s.email);
			emailStmt->setString(2, s.studentName);
			emailStmt->setString(3, b.title);
			emailStmt->setInt(4, b.bookId);
			emailStmt->setString(5, currentDate);
			emailStmt->setInt(6, fine);

			emailStmt->executeUpdate();
			cout << "Return email added to queue.\n";
		}
		catch (sql::SQLException& e) {
			cout << "Error inserting into return_email_queue: " << e.what() << endl;
		}

	}

	void showIssueRecord(sql::Connection* con)
	{
		try {
			unique_ptr<sql::Statement> stmt(con->createStatement());
			unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM issued_books"));
			bool hasRecords = false;
			while (res->next()) {
				if (!hasRecords) {
					cout << "\n\t\t\t -- ISSUED BOOKS RECORD ---\n";
					hasRecords = true;
				}
				
				cout << "\n---------------------------------------------" << endl;
				cout << "BOOK ID	: " << res->getInt("book_id") << endl;
				cout << "BOOK TITLE : " << res->getString("book_title") << endl;
				cout << "STUDENT NAME : " << res->getString("student_name") << endl;
				cout << "FATHER NAME : " << res->getString("father_name") << endl;
				cout << "ROLL NUMBER : " << res->getInt("roll_number") << endl;
				cout << "SECTION :" << res->getString("section") << endl;
				cout << "EMAIL : " << res->getString("email") << endl;
				cout << "\n---------------------------------------------" << endl;
			}
			if (!hasRecords) {
				cout << "\n\t\t\t[!] No books are issued right now.\n";
			}
		}
		catch (sql::SQLException& e) {
			cout << "Error viewing books: " << e.what() << endl;
		}
	}
private:
	string getCurrentDate() {
		auto now = chrono::system_clock::now();
		time_t now_time = chrono::system_clock::to_time_t(now);

		tm bt;
		localtime_s(&bt, &now_time);

		ostringstream oss;
		oss << put_time(&bt, "%Y-%m-%d");
		return oss.str();
	}

	string calculateDueDate(int days) {
		auto now = chrono::system_clock::now();

		auto future_date = now + chrono::hours(24 * days);

		time_t future_time = chrono::system_clock::to_time_t(future_date);
		tm bt;
		localtime_s(&bt, &future_time);

		ostringstream oss;
		oss << put_time(&bt, "%Y-%m-%d");
		return oss.str();
	}

	void generateReceiptHTML(const Student& s, const Book& b, const string& baseName)
	{
		string fileName = baseName + ".html";
		ofstream html("book/" + fileName);
		if (!html)
		{
			cout << "Error: Ensure the 'book' folder exists!\n";
			return;
		}

		html << "<!DOCTYPE html><html><head><meta charset='UTF-8'></head>\n";
		html << "<body style='margin:0; padding:20px; background-color:#f4f4f4; font-family:\"Segoe UI\",Arial,sans-serif;'>\n";

		// Main Container
		html << "<div style='max-width:500px; margin:auto; background:#ffffff; border-radius:8px; overflow:hidden; box-shadow:0 4px 10px rgba(0,0,0,0.1); border:1px solid #ddd;'>\n";

		// Header
		html << "<div style='background:#002147; color:#ffffff; padding:20px; text-align:center;'>\n";
		html << "<h2 style='margin:0; letter-spacing:1px;'>LIBRARY RECEIPT</h2>\n";
		html << "</div>\n";

		// Body Content
		html << "<div style='padding:20px;'>\n";

		// Student Section
		html << "<p style='color:#002147; font-weight:bold; border-bottom:2px solid #FFCC00; display:inline-block; margin-bottom:10px;'>STUDENT DETAILS</p>\n";
		html << "<table style='width:100%; font-size:14px; color:#333;'>\n";
		html << "<tr><td style='padding:5px 0;'><b>Name:</b></td><td>" << s.studentName << "</td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Roll No:</b></td><td>" << s.rollNumber << "</td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Section:</b></td><td>" << s.section << "</td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Father Name:</b></td><td>" << s.fatherName << "</td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Email:</b></td><td>" << s.email << "</td></tr>\n";
		html << "</table>\n";

		html << "<br>\n";

		// Book Section
		html << "<p style='color:#002147; font-weight:bold; border-bottom:2px solid #FFCC00; display:inline-block; margin-bottom:10px;'>BOOK DETAILS</p>\n";
		html << "<table style='width:100%; font-size:14px; color:#333;'>\n";
		html << "<tr><td style='padding:5px 0;'><b>Title:</b></td><td>" << b.title << "</td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Author:</b></td><td>" << b.author << "</td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Book ID:</b></td><td>" << b.bookId << "</td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Issue Date:</b></td><td style='color:green;'><b>" << s.issueDate << "</b></td></tr>\n";
		html << "<tr><td style='padding:5px 0;'><b>Due Date:</b></td><td style='color:red;'><b>" << s.dueDate << "</b></td></tr>\n";
		html << "</table>\n";

		html << "</div>\n"; // End Padding

		// Footer
		html << "<div style='background:#f9f9f9; padding:15px; text-align:center; font-size:12px; color:#777; border-top:1px solid #eee;'>\n";
		html << "Please return the book by the due date to avoid fines.<br>\n";
		html << "<b>� 2026 Library Management System</b>\n";
		html << "</div>\n";

		html << "</div></body></html>\n";
		html.close();

		cout << "Professional Receipt generated: book/" << fileName << endl;
	}

	void fetchBookDetails(sql::Connection* con, Book& b) {
		try {
			unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
				"SELECT title, author FROM books WHERE book_id = ?"
			));
			pstmt->setInt(1, b.bookId);
			unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

			if (res->next()) {
				b.title = res->getString("title");
				b.author = res->getString("author");
			}
			else {
				cout << "Book ID not found in database!" << endl;
			}
		}
		catch (sql::SQLException& e) {
			cout << "SQL Error: " << e.what() << endl;
		}
	}

	void convertHTMLtoPDF(const string& baseFileName) {
		// Since this code is called from main (root), 
		// we keep the "book/" prefix to target the subfolder.
		string htmlFilename = "book/" + baseFileName + ".html";
		string pdfFilename = "book/" + baseFileName + ".pdf";

		// Check if HTML exists
		if (!fs::exists(htmlFilename)) {
			cout << "Error: Cannot find " << htmlFilename << " for conversion." << endl;
			return;
		}

		// Get Absolute Paths (Best for wkhtmltopdf)
		string htmlPath = fs::absolute(htmlFilename).string();
		string pdfPath = fs::absolute(pdfFilename).string();

		// Fix Windows slashes for the command line
		for (char& c : htmlPath) if (c == '\\') c = '/';
		for (char& c : pdfPath) if (c == '\\') c = '/';

		// Construct command with triple slashes for file protocol
		string cmd = "wkhtmltopdf --quiet --enable-local-file-access "
			"\"file:///" + htmlPath + "\" "
			"\"" + pdfPath + "\"";

		// Execute
		int result = system(cmd.c_str());

		if (result == 0) {
			cout << "PDF Created successfully in book/ folder." << endl;
		}
		else {
			cout << "Conversion failed. Please check your wkhtmltopdf installation." << endl;
		}
	}
	sql::Connection* con;
	bool isBookIssued(int bookId, int rollNumber, string section) {
		try {
			unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
				"SELECT count(*) FROM issued_books WHERE book_id = ? AND roll_number = ? AND section = ?"
			));
			pstmt->setInt(1, bookId);
			pstmt->setInt(2, rollNumber);
			pstmt->setString(3, section);
			unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
			if (res->next()) {
				return res->getInt(1) > 0;
			}
		}
		catch (sql::SQLException& e) {
			cout << "SQL Error: " << e.what() << endl;
		}
		return false;
	}

	int dateToDays(string dateStr) {
		struct tm tm = { 0 };
		istringstream ss(dateStr);

		ss >> get_time(&tm, "%Y-%m-%d");

		time_t time = mktime(&tm);

		return time / (24 * 60 * 60);
	}


	bool getIssueDetailsByBookId(int bookId, int roll, string sec, Student& s, Book& b) {
		try {
			unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
				"SELECT student_name, roll_number, section, email, father_name, book_title, issue_date, due_date "
				"FROM issued_books WHERE book_id = ? AND roll_number = ? AND section = ?"
			));
			pstmt->setInt(1, bookId);
			pstmt->setInt(2, roll);
			pstmt->setString(3, sec);
			unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

			if (res->next()) {
				s.studentName = res->getString("student_name");
				s.rollNumber = res->getInt("roll_number");
				s.section = res->getString("section");
				s.email = res->getString("email");
				s.fatherName = res->getString("father_name");
				b.title = res->getString("book_title");
				b.bookId = bookId;

				s.issueDate = res->getString("issue_date");
				s.dueDate = res->getString("due_date");

				return true;
			}
		}
		catch (sql::SQLException& e) {
			cout << "Error in fetching data from issued_books" << e.what() << endl;
		}
		return false;
	}
};
#endif