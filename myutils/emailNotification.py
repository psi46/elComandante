import os
import smtplib
 
from email import Encoders
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.MIMEMultipart import MIMEMultipart
from email.Utils import formatdate
 
filePath = 'testMail.txt'
class EmailNotification(object):
    def __init__(self,HOST,TO,FROM):
        self.host = HOST
        self.recipient = TO
        self.sender = FROM

    def sendEmail(self,subject ='EmailNotification',msg='Test'):
        mail = MIMEText(msg)
        mail["From"] = self.sender
        mail["To"] = self.recipient
        mail["Subject"] = subject
        mail['Date']    = formatdate(localtime=True)
        try:
            server = smtplib.SMTP(self.host)
            print 'SEND MAIL!\n*******\n'
            #print '\n',msg.as_string(),'\n******\n'
            failed = server.sendmail(self.sender,self.recipient, mail.as_string())
            print 'STATUS: ',failed
            server.close()
        except Exception as e:
            errorMsg = "Unable to send email. Error: %s" % str(e)
            print errorMsg
            raise e
     
if __name__ == "__main__":
    msg = EmailNotification(HOST='mail.phys.ethz.ch',TO='0787147402@sms.ethz.ch',FROM='testmail@phys.ethz.ch')
    msg.sendEmail(subject="What is this?",msg='Can you pleas update the plots?\nCheers Rainer')
    #sendEmail()
