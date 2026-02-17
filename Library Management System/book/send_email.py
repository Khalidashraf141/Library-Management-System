import smtplib
import os
import sys
from email.message import EmailMessage

# Arguments from C++
receiver_email = sys.argv[1]
pdf_base_name  = sys.argv[2]   # without .pdf

SENDER_EMAIL = os.environ.get("LIB_EMAIL_USER")
APP_PASSWORD = os.environ.get("LIB_EMAIL_PASS")

if not SENDER_EMAIL or not APP_PASSWORD:
    print("Email failed: Missing environment varialbes.")
    sys.exit(1)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PDF_FILE = os.path.join(BASE_DIR, pdf_base_name + ".pdf")

if not os.path.exists(PDF_FILE):
    print(f"Error: PDF file not found: {PDF_FILE}")
    sys.exit(1)

msg = EmailMessage()
msg["Subject"] = "Library Book Issue Receipt"
msg["From"] = SENDER_EMAIL
msg["To"] = receiver_email

msg.set_content(
    "Dear Student,\n\n"
    "Your book has been issued successfully.\n"
    "Please find the receipt attached.\n\n"
    "Regards,\n"
    "Library Management System"
)

try:
    with open(PDF_FILE, "rb") as f:
        msg.add_attachment(
            f.read(),
            maintype="application",
            subtype="pdf",
            filename=os.path.basename(PDF_FILE)
        )

    with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
        server.login(SENDER_EMAIL, APP_PASSWORD)
        server.send_message(msg)

    print("Email sent successfully")
    sys.exit(0)

except Exception as e:
    print("Failed to send email:", e)
    sys.exit(1)