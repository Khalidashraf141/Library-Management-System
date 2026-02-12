# 📚 Library Management System

![C++](https://img.shields.io/badge/Language-C++-blue)
![MSVC](https://img.shields.io/badge/Compiler-MSVC-green)
![MySQL](https://img.shields.io/badge/Database-MySQL-orange)
![License](https://img.shields.io/badge/License-MIT-yellow)

A complete **Library Management System** developed using **C++ (MSVC)** with **MySQL database integration**, automated **PDF receipt generation**, and **email notification functionality**.

This project demonstrates strong backend development concepts including database connectivity, structured programming, file handling, external tool integration, and modular project design.

---

## 🚀 Features

- 👤 Admin Authentication System  
- 📚 Add / Update / Delete Books  
- 👨‍🎓 Issue and Return Books  
- 🗃 MySQL Database Integration  
- 🧾 Automatic Receipt Generation (HTML → PDF)  
- 📧 Email Receipt with PDF Attachment  
- 🔐 Secure Input Handling  
- 📊 Record Management System  

---

## 🛠️ Technologies Used

| Technology        | Purpose                          |
|------------------|----------------------------------|
| C++ (MSVC)       | Core Backend Logic               |
| MySQL            | Database Storage                 |
| MySQL Connector  | C++ Database Connectivity        |
| HTML             | Receipt Template Design          |
| wkhtmltopdf      | HTML to PDF Conversion           |
| Python           | Sending Email with Attachment    |

---

## 📂 Project Structure

```
Library-Management-System/
│
├── src/
│   ├── main.cpp
│   ├── admin/
│   ├── user/
│
├── database/
│   └── schema.sql
│
├── receipts/
│   ├── receipt.html
│   └── generated_receipt.pdf
│
├── email/
│   └── send_mail.py
│
├── .gitignore
├── README.md
└── LICENSE
```

---

## ⚙️ Installation & Setup

### 1️⃣ Clone the Repository

```bash
git clone https://github.com/yourusername/Library-Management-System.git
cd Library-Management-System
```

---

### 2️⃣ Install MySQL

Download and install MySQL Server.

Create database:

```sql
CREATE DATABASE library_db;
```

Import schema:

```bash
mysql -u root -p library_db < database/schema.sql
```

---

### 3️⃣ Configure Database Connection (C++)

Update credentials inside your connection file:

```cpp
const char* host = "localhost";
const char* user = "root";
const char* password = "yourpassword";
const char* db = "library_db";
```

---

### 4️⃣ Install MySQL Connector/C++

- Download MySQL Connector/C++
- Configure include directories in Visual Studio
- Link required `.lib` files in project settings

---

### 5️⃣ Install wkhtmltopdf

Download from:

https://wkhtmltopdf.org/downloads.html

After installation, ensure it is added to your system PATH.

Test installation:

```bash
wkhtmltopdf --version
```

---

### 6️⃣ Setup Python Email Script

Install required library:

```bash
pip install secure-smtplib
```

Edit credentials inside:

```
email/send_mail.py
```

> ⚠️ Do not upload real email passwords to GitHub.

---

### 7️⃣ Build Using MSVC (Visual Studio)

1. Open solution in Visual Studio  
2. Configure include & library directories  
3. Link MySQL Connector  
4. Build Solution  

---

## 🧾 Receipt Generation Workflow

1. C++ generates a dynamic HTML receipt.
2. `wkhtmltopdf` converts HTML to PDF.
3. Python script sends the generated PDF to the user via email.

---

## 🔐 Security Recommendations

- Use environment variables for database credentials
- Use App Password for email authentication
- Add sensitive configuration files to `.gitignore`
- Avoid hardcoding passwords in source code

---

## 📈 Future Improvements

- 🌐 Web-based frontend version
- 🔑 Role-based access control
- 💰 Fine calculation automation
- 🖥️ GUI Desktop version
- ☁️ Cloud deployment support
- 📊 Admin dashboard analytics

---

## 📸 Optional: Add Screenshots

You can add screenshots like this:

```md
## Screenshots

![Dashboard](screenshots/dashboard.png)
![Receipt](screenshots/receipt.png)
```

---

## 🤝 Contribution

Contributions are welcome!

1. Fork the repository  
2. Create a new feature branch  
3. Commit your changes  
4. Push to your branch  
5. Create a Pull Request  

---

## 📄 License

This project is licensed under the **MIT License**.

---

## 👨‍💻 Author

Developed by **FRANCHISE**

---

## ⭐ Support

If you found this project helpful, please consider giving it a ⭐ on GitHub!
