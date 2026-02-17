#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include <chrono>
#include <limits>
#include <memory>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>

#include "book/book.h"
#include "admin/login.h"


using namespace std;


namespace {
	int safeInput() {
		int x;
		while (!(cin >> x)) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Invalid input! Please enter a number: ";
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		return x;
	}


	unique_ptr<sql::Connection> connectDB() {
		try {
			sql::Driver* driver = get_driver_instance();
			unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "root"));
			con->setSchema("library_db");
			return con;
		}
		catch (sql::SQLException& e) {
			cout << "DB Connection Error: " << e.what() << endl;
			return nullptr;
		}
	}
}

void emailWorker() {
	while (true) {
		try {
			auto con = connectDB();
			if (!con) {
				this_thread::sleep_for(chrono::seconds(10));
				continue;
			}

			unique_ptr<sql::PreparedStatement>pstmt(
				con->prepareStatement(
					"SELECT id, student_email, receipt_name FROM email_queue WHERE status = 'PENDING'"
				)
			);
			unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

			while (res->next()) {
				int id = res->getInt("id");
				string email = res->getString("student_email");
				string receipt = res->getString("receipt_name");

				string cmd = "python book/send_email.py \"" + email + "\" \"" + receipt + "\"";

				if (system(cmd.c_str()) == 0) {
					unique_ptr<sql::PreparedStatement> updateStmt(
						con->prepareStatement(
							"UPDATE email_queue SET status = 'SENT' WHERE id = ?"
						)
					);
					updateStmt->setInt(1, id);
					updateStmt->executeUpdate();
					cout << "[Auto Email Sent] " << email << endl;
				}
			}
		}
		catch (...) {
			cout << "Error";
		}
		this_thread::sleep_for(chrono::seconds(10));
	}
}

void returnEmailWorker() {
	while (true) {
		try {
			auto con = connectDB();
			if (!con) {
				this_thread::sleep_for(chrono::seconds(10));
				continue;
			}
			unique_ptr<sql::PreparedStatement>pstmt(
				con->prepareStatement(
					"SELECT id, student_email, student_name, book_title, book_id, return_date, fine_amount "
					"FROM return_email_queue WHERE status = 'PENDING'"
				)
			);
			unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

			while (res->next()) {
				int id = res->getInt("id");
				string email = res->getString("student_email");
				string student = res->getString("student_name");
				string title = res->getString("book_title");
				string bookId = to_string(res->getInt("book_id"));
				string date = res->getString("return_date");
				string fine = to_string(res->getInt("fine_amount"));


				string cmd = "python book/return_email.py \"" + email + "\" \"" +
					student + "\" \"" + title + "\" \"" +
					bookId + "\" \"" + date + "\" \"" + fine + "\"";

				if (system(cmd.c_str()) == 0) {
					unique_ptr<sql::PreparedStatement> updateStmt(
						con->prepareStatement(
							"UPDATE return_email_queue SET status = 'SENT' WHERE id = ?"
						)
					);
					updateStmt->setInt(1, id);
					updateStmt->executeUpdate();

					cout << "[Auto Return Email Sent] " << email << endl;
				}

			}
		}
		catch (...) {
			cout << "Return email worker error\n";
		}

	this_thread::sleep_for(chrono::seconds(10));
	}
}


int main() {
	bool libraryRunning = true;

	auto con = connectDB();
	if (!con) return 1;

	
	thread worker(emailWorker);
	worker.detach();
	thread returnWorker(returnEmailWorker);
	returnWorker.detach();


	while (libraryRunning)
	{
		cout << "\n======================================\n"
			<< "     Library Management System       \n"
			<< "======================================\n";
		cout << "1. Admin\n2. Student\n3. Exit\nEnter a Choice: ";

		switch (safeInput())
		{
		case 1:
		{
			if (!con || con->isClosed()) {
				cout << "Database connection lost. Attempting to reconnect...\n";
				con = connectDB();
				if (!con) {
					cout << "Reconnection is failed check your MySQL service\n";
					break;
				}
			}
			Login l;
			if (l.admin(con.get()))
			{
				cout << "--- Access Granted ---" << endl;
				bool adminRunning = true;
				LibraryBook lb;

				while (adminRunning)
				{
					cout << "\n--- Admin Panel ---\n";
					cout << "1. Add Book\n2. Show all Books\n3. Find Book\n4. Issue Book\n5. Return Book\n6. Show Issue Book Record\n7. Exit Admin Panel\nEnter your choice: ";
					switch (safeInput())
					{
					case 1: lb.addBook(con.get()); break;
					case 2: lb.viewBooks(con.get()); break;
					case 3: lb.findBook(con.get()); break;
					case 4: lb.issueBook(con.get()); break;
					case 5: lb.returnBook(con.get()); break;
					case 6: lb.showIssueRecord(con.get()); break;
					case 7: adminRunning = false; break;
					default: cout << "\nInvalid choice!";
					}
				}
			}
			break;
		}
		case 2: cout << "Student Panel\n"; break;
		case 3: libraryRunning = false; break;
		default: cout << "\nInvalid choice! Please select 1 or 2.\n";
		}
	}
	return 0;
}
