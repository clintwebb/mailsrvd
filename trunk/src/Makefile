# //-----------------------------------------------------------------------------
# // Mailsrvd
# //  (c) Copyright Hyper-Active Systems, 2006.
# //
# // Hyper-Active Systems
# // ABN: 98 400 498 123
# // 66 Brown Cr
# // Seville Grove, WA, 6112
# // Australia
# // Phone: +61 434 895 695
# //        0434 895 695
# // contact@hyper-active.com.au
# // 
# //-----------------------------------------------------------------------------
#
# makefile

OBJS=Session.o Server.o Logger.o \
	 SmtpServer.o PopServer.o \
	 Message.o PopSession.o SenderSession.o
LIBS=-lsqlite3 -lpthread -ldevplus -ldevplus-thread -ldevplus-sqlite

#CONFIG_DIR="/etc"
#LOG_DIR="/var/log/mailsrvd"

H_Defaults=Defaults.h
H_Logger=Logger.h
H_Session=Session.h $(H_Logger)
H_Message=Message.h $(H_Session)
H_PopSession=PopSession.h $(H_Session)
H_Server=Server.h $(H_Session) $(H_Logger)
H_PopServer=PopServer.h $(H_Server)
H_SmtpServer=SmtpServer.h $(H_Server)
H_SenderSession=SenderSession.h $(H_Session)
H_Sender=Sender.h $(H_SenderSession)


all: mailsrv-pop3 mailsrv-smtp mailsrv-out

mailsrv-pop3: main-pop3.cpp $(OBJS) $(H_Defaults) $(H_Logger) $(H_SenderSession) $(H_PopServer)  
	g++ -o $@ main-pop3.cpp $(OBJS) $(LIBS)

mailsrv-smtp: main-smtp.cpp $(OBJS) $(H_Defaults) $(H_Logger) $(H_SenderSession) $(H_SmtpServer)  
	g++ -o $@ main-smtp.cpp $(OBJS) $(LIBS) 

mailsrv-out: main-out.cpp   $(OBJS) $(H_Defaults) $(H_Logger) $(H_SenderSession) $(H_SmtpServer)  
	g++ -o $@ main-out.cpp  $(OBJS) $(LIBS) 

PopServer.o: PopServer.cpp $(H_Server) $(H_PopSession)
	g++ -Wall -c -o $@ PopServer.cpp

Logger.o: Logger.cpp $(H_Logger)
	g++ -Wall -c -o $@ Logger.cpp

Server.o: Server.cpp $(H_Server)
	g++ -Wall -c -o $@ Server.cpp

SmtpServer.o: SmtpServer.cpp $(H_SmtpServer) $(H_Message)
	g++ -Wall -c -o $@ SmtpServer.cpp

Message.o: Message.cpp $(H_Message)
	g++ -Wall -c -o $@ Message.cpp 

Session.o: Session.cpp $(H_Session)
	g++ -Wall -c -o $@ Session.cpp

PopSession.o: PopSession.cpp $(H_PopSession)
	g++ -Wall -c -o $@ PopSession.cpp

SenderSession.o: SenderSession.cpp $(H_SenderSession)
	g++ -Wall -c -o $@ SenderSession.cpp



clean:
	@-rm $(OBJS)


