#ifndef LOGIN_H
#define LOGIN_H
#include <iostream>
#include <limits>
#include <conio.h>
#include <string>
#include <memory> // For unique_ptr
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
using namespace std;

class Login
{
public:
	bool admin(sql::Connection* con)
	{
		string adminUser;
		string adminPass;

		cout << "Enter username : ";
		cin >> adminUser;

		cout << "Enter password : ";
		adminPass = getPassword();

		try {
			unique_ptr<sql::PreparedStatement>pstmt(con->prepareStatement(
				"SELECT * FROM system_admins WHERE username = ? AND password = ?"));


			//Bind the user input to the '?' placeholders
			pstmt->setString(1, adminUser);
			pstmt->setString(2, adminPass);

			unique_ptr<sql::ResultSet>res(pstmt->executeQuery());

			if (res->next()) {
				cout << "Login sucessful!" << endl;
				return true;
			}
			else {
				cout << "Login failed! Incorrect username and password." << endl;
				return false;
			}
		}
		catch (sql::SQLException& e) {
			cout << "Database Error: " << e.what() << endl;
			return false;
		}

	}

private:
	string getPassword()
	{
		string pass;
		char ch;
		while (true)
		{
			ch = _getch();
			if (ch == 13)
			{
				cout << endl;
				break;
			}
			else if (ch == 8)
			{
				if (!pass.empty()) {
					pass.pop_back();
				}
			}
			else {
				pass.push_back(ch);
			}
		}
		return pass;
	}
};
#endif