import sys
import smtplib
import os
from email.message import EmailMessage


to_email = sys.argv[1]
student_name = sys.argv[2]
book_title = sys.argv[3]
book_id = sys.argv[4]
return_date = sys.argv[5]
fine = sys.argv[6]

EMAIL_ADDRESS = os.environ.get("LIB_EMAIL_USER")
EMAIL_PASSWORD = os.environ.get("LIB_EMAIL_PASS")

if not EMAIL_ADDRESS or not EMAIL_PASSWORD:
    print("Email failed: Missing environment variables.")
    sys.exit(1)

msg = EmailMessage()
msg["Subject"] = "Library Book Return Confirmation"
msg["From"] = EMAIL_ADDRESS
msg["To"] = to_email

# Email body (with or without fine)
if fine == "0":
    msg.set_content(f"""
Hello {student_name},

Your library book has been returned successfully.

Book Title  : {book_title}
Book Id     : {book_id}
Return Date : {return_date}

Thank you for returning the book on time.
We appreciate your cooperation.

Regards,
Library Management System
""")
else:
    msg.set_content(f"""
Hello {student_name},

Your library book has been returned successfully.

Book Title  : {book_title}
Book ID     : {book_id} 
Return Date : {return_date}
Fine Amount : ₹{fine}

Please ensure timely returns in the future
to avoid fines.

Regards,
Library Management System
""")

# 📤 Send email
try:
    with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
        server.login(EMAIL_ADDRESS, EMAIL_PASSWORD)
        server.send_message(msg)
        print("Email sent successfully")
        sys.exit(0)
except Exception as e:
    sys.exit(1)