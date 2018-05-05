#!/usr/bin/env python
import smtplib
from email.mime.text import MIMEText

USERNAME = "riccardo.ancona@gmail.com"
PASSWORD = "supbtqkuszearsql"
MAILTO  = "riccardo.ancona@gmail.com"

msg = MIMEText('Hello,\nMy name is ArduinoCrono, \n the water level is too low ')
msg['Subject'] = 'Watering System - Water Level is too low'
msg['From'] = USERNAME
msg['To'] = MAILTO

server = smtplib.SMTP('smtp.gmail.com:587')
server.ehlo_or_helo_if_needed()
server.starttls()
server.ehlo_or_helo_if_needed()
server.login(USERNAME,PASSWORD)
server.sendmail(USERNAME, MAILTO, msg.as_string())
server.quit()
